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
