# Bruno's Awful Examples (bae)

Used to be Bruno's Awful Engine, but decided it was better to focus on small little examples, like what is found in the bgfx repo

## Getting started

```
git clone git@github.com:BruOp/bae.git
cd bae
git submodule update --init --recursive
```

Windows

```
make setup
```

You'll also need to build `shaderc` (it should be part of the generated solution) and then copy it to `deps/bgfx/tools/bin/windows/shaderc` (or whatever platform you're using). This will elt you build the shaders inside each example using the makefile.

You may need to install all the system-level dependencies for `bgfx` on your own though :( Follow the bgfx build instructions for this part. This repo doesn't introduce any new system level deps.

## Linux

On linux, you'll probably have to download a whole bunch of `devel` packages. Install the ones for bgfx listed [here](https://bkaradzic.github.io/bgfx/build.html). For Solus Distro, the packages names were totally different, so I ran:

```zsh
sudo eopkg install libglu libglu-devel libx11 libx11-devel xorg-server xorg-server-devel
```

## Using GLTF files

There's an GLTF loader in `PhysicallyBasedScene.cpp` that doesn't support the spec fully. Additionally, since BGFX doesn't expose any way of generating mip maps for textures a la `glGenMipmaps`, we have to pre-process the GLTF files to produce mip-mapped DDS versions of the source textures.

To pre-process the GLTFs you'll need `python` 3.7 or greater installed (sorry), and then you can run the following from the project root:

```bash
python3 scripts/convert_textures.py path/to/Source/GLTF/file.gltf examples/runtime/meshes/output_dir
```
