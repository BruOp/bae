# bae

Meant to eventually be a little demo engine/renderer using bgfx, imgui, glfw and glm.

Inspired by [bigg](https://github.com/JoshuaBrookover/bigg)'s handling of all the dependencies.

## Getting started

```
git clone git@github.com:BruOp/bae.git
cd bae
git submodule update --init --recursive
mkdir build
cd build
cmake ..
```

This will generate the project file/make files that you need for `bae` + all the dependencies.

You may need to install all the system-level dependencies for `bgfx` on your own though :(

## Linux

On linux, you'll probably have to download a whole bunch of `devel` packages. Install the ones for bgfx listed [here](https://bkaradzic.github.io/bgfx/build.html). For Solus Distro, the packages names were totally different, so I ran:

```zsh
sudo eopkg install libglu libglu-devel libx11 libx11-devel xorg-server xorg-server-devel
```

I had some difficulty the headers required for GLFW, but that was solved by simply running

```zsh
sudo eopkg install glfw-devel libxi libxi-devel
```

Unfortunately, you'll have to play a bit of whack-a-mole to resolve all these problems.
