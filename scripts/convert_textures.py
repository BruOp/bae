import argparse
import subprocess
from pathlib import Path


OUTPUT_SUFFIX = '.dds'


def process_folder(folder_path, texturec_path):
    path = Path(folder_path)
    texturec_tool = Path(texturec_path)
    assert path.exists()
    assert texturec_tool.exists()

    diffuse_files = list(path.glob('*_diffuse*'))
    diffuse_files.extend(path.glob('*_baseColor*'))
    diffuse_files.extend(path.glob('*_albedo*'))

    process_files(texturec_tool, diffuse_files, type='RGBA8')

    normal_maps = list(path.glob('*_normal*'))
    process_files(texturec_tool, normal_maps, type='RGBA8', is_linear=True, is_normal=True)

    pbr_maps = list(path.glob('*_metallicRoughness*'))
    pbr_maps.extend(path.glob('*_metalRoughness*'))
    pbr_maps.extend(path.glob('*_occlusionRoughnessMetallic*'))
    process_files(texturec_tool, pbr_maps, type='RGBA8', is_linear=True)


def process_files(texturec_tool, files, type=None, is_linear=False, is_normal=False, additional_args=None):
    files = [f for f in files if f.suffix != OUTPUT_SUFFIX]
    for file in files:
        print('Parsing: ' + str(file))
        output = file.with_suffix(OUTPUT_SUFFIX)
        args = [str(texturec_tool), '-f', str(file), '-o', str(output)]

        if type is not None:
            args = args + ['-t', type, '-m']
        else:
            args.append('-m')


        if is_linear:
            args.append('--linear')

        if is_normal:
            args.append('-n')

        if additional_args is not None:
            args = args + additional_args

        print(" ".join(args))
        subprocess.run(args, check=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'asset_folder',
        help='The path to the folder containing texture assets you wish to convert'
    )
    parser.add_argument(
        '-t',
        '--texturec_path',
        help='The path to the texturec.exe executable tool',
        default='./tools/texturec.exe'
    )
    args = parser.parse_args()
    print(args.texturec_path)
    process_folder(args.asset_folder, args.texturec_path)
