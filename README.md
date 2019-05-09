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
