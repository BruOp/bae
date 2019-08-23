import argparse
import json
import shutil
import subprocess
import logging
from dataclasses import dataclass
from pathlib import Path


OUTPUT_SUFFIX = '.dds'

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.DEBUG)


def get(json_dict, *keys):
    acc = json_dict
    for key in keys:
        acc = acc.get(key)
        if acc is None:
            return
    return acc


@dataclass
class TextureConversionCommand:
    file_path: Path
    is_linear: bool = False
    is_normal: bool = False


@dataclass
class EditCommand:
    image_idx: int
    new_uri: str


class GltfTextureConverter:
    def __init__(self, gltf_file_path, output_path, texturec_path):
        self.gltf_path = Path(gltf_file_path)
        self.output_path = Path(output_path)
        self.texturec_tool = Path(texturec_path)
        assert self.gltf_path.exists()
        assert self.texturec_tool.exists()

        with open(self.gltf_path, 'r') as f:
            self.gltf_json = json.load(f)

        self.conversion_commands: list[TextureConversionCommand] = []
        self.edit_commands: list[EditCommand] = []

    def process(self):
        self.output_path.mkdir(exist_ok=True)
        self._generate_commands()
        self._process_gltf_edit_commands()
        self._process_texture_commands()
        binary_files = self.gltf_path.parent.glob('**/*.bin')
        for bin_file in binary_files:
            shutil.copy2(bin_file, dst=self.output_path)

    def _generate_commands(self):
        materials = self.gltf_json['materials']
        for i, material in enumerate(materials):
            pbrProperties = material.get('pbrMetallicRoughness')

            if pbrProperties is None:
                logger.warning(f'Material with index {i} does not have PBR properties')
                continue

            texture_idx = get(pbrProperties, 'baseColorTexture', 'index')
            self._generate_command(texture_idx, is_linear=False, is_normal=False)

            texture_idx = get(pbrProperties, 'metallicRoughnessTexture', 'index')
            self._generate_command(texture_idx, is_linear=True, is_normal=False)

            texture_idx = get(material, 'normalTexture', 'index')
            self._generate_command(texture_idx, is_linear=True, is_normal=True)

            texture_idx = get(material, 'occlusionTexture', 'index')
            self._generate_command(texture_idx, is_linear=True, is_normal=False)

            texture_idx = get(material, 'emissiveTexture', 'index')
            self._generate_command(texture_idx, is_linear=False, is_normal=False)

    def _generate_command(self, texture_idx, is_linear, is_normal):
        if texture_idx is None:
            return

        # Create command, append it to list.
        texture_dict = self.gltf_json['textures'][texture_idx]
        img_uri = self.gltf_json['images'][texture_dict['source']]['uri']
        image_file_path = self.gltf_path.parent / img_uri

        self.conversion_commands.append(TextureConversionCommand(
            file_path=image_file_path,
            is_linear=is_linear,
            is_normal=is_normal,
        ))
        self.edit_commands.append(EditCommand(
            image_idx=texture_dict['source'],
            new_uri=str(Path(img_uri).with_suffix(OUTPUT_SUFFIX)),
        ))

    def _process_texture_commands(self):
        for command in self.conversion_commands:
            input_file = str(command.file_path)
            logger.info('Parsing: ' + input_file)
            output = str((self.output_path / command.file_path.name).with_suffix(OUTPUT_SUFFIX))
            args = [str(self.texturec_tool), '-f', input_file, '-o', output, '-t', 'RGBA8', '-m']

            if command.is_linear:
                args.append('--linear')

            if command.is_normal:
                args.append('-n')

            logger.info('Using arguments: ' + ' '.join(args))
            subprocess.run(args, check=True)

    def _process_gltf_edit_commands(self):
        images = self.gltf_json['images']
        for idx, command in enumerate(self.edit_commands):
            images[command.image_idx]['uri'] = command.new_uri

        new_gltf_path = self.output_path.joinpath(self.gltf_path.name)
        with open(new_gltf_path, 'x') as f:
            json.dump(self.gltf_json, f)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'gltf_path',
        help='The path to the folder containing texture assets you wish to convert',
    )

    parser.add_argument(
        'output_path',
        help='The path where you would like to place the modified files',
    )
    parser.add_argument(
        '-t',
        '--texturec_path',
        help='The path to the texturec.exe executable tool',
        default='./tools/texturec.exe',
    )
    args = parser.parse_args()
    converter = GltfTextureConverter(args.gltf_path, args.output_path, args.texturec_path)
    converter.process()
