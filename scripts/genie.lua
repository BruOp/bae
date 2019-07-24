--
-- Copyright 2010-2019 Branimir Karadzic. All rights reserved.
-- License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
--

newoption {
    trigger = "with-wayland",
    description = "Use Wayland backend."
}

newoption {
    trigger = "with-profiler",
    description = "Enable build with intrusive profiler."
}

newoption {
    trigger = "with-combined-examples",
    description = "Enable building examples (combined as single executable)."
}

newoption {
    trigger = "with-examples",
    description = "Enable building examples."
}

BAE_DIR = (path.getabsolute("..") .. "/")
DEPENDENCY_DIR = (BAE_DIR .. "deps/")
EXAMPLES_DIR = (BAE_DIR .. "examples/")

BGFX_DIR = path.join(DEPENDENCY_DIR, "bgfx/")
BX_DIR = path.join(DEPENDENCY_DIR, "bx/")
BIMG_DIR = path.join(DEPENDENCY_DIR, "bimg/")

EXTERNAL_DIR = (BAE_DIR .. "external/")
GLM_DIR = path.join(EXTERNAL_DIR, "glm/")
ASSIMP_DIR = path.join(EXTERNAL_DIR, "assimp/")

local BGFX_SCRIPTS_DIR = (DEPENDENCY_DIR .. "bgfx/scripts/")
local BUILD_DIR = path.join(BAE_DIR, ".build")
local BGFX_THIRD_PARTY_DIR = path.join(BGFX_DIR, "3rdparty")

--
-- Solution
--
solution "bae"
language "C++"
configurations {"Debug", "Release"}
platforms {"x32", "x64"}
startproject "bae"

-- Dependencies

dofile(path.join(BX_DIR, "scripts/toolchain.lua"))
if not toolchain(BUILD_DIR, BGFX_THIRD_PARTY_DIR) then
    return -- no action specified
end

function copyLib()
end

if _OPTIONS["with-wayland"] then
    defines {"WL_EGL_PLATFORM=1"}
end

if _OPTIONS["with-profiler"] then
    defines {
        "ENTRY_CONFIG_PROFILER=1",
        "BGFX_CONFIG_PROFILER=1"
    }
end

function exampleProjectDefaults()
    debugdir(path.join(EXAMPLES_DIR, "runtime"))

    includedirs {
        path.join(BX_DIR, "include"),
        path.join(BIMG_DIR, "include"),
        path.join(BGFX_DIR, "include"),
        path.join(BGFX_DIR, "3rdparty"),
        path.join(BGFX_DIR, "examples/common"),
        path.join(GLM_DIR, "include"),
        path.join(BAE_DIR, "include"),
        -- path.join(TINYOBJ_DIR, "include"),
        -- path.join(TINYGLTF_DIR, "include"),
        ASSIMP_DIR
    }

    flags {
        "FatalWarnings"
    }

    defines {
        "_HAS_ITERATOR_DEBUGGING=0",
        "_SECURE_SCL=0"
    }

    libdirs {ASSIMP_DIR}

    links {
        "assimp-vc140-mt",
        "example-common",
        "example-glue",
        "bgfx",
        "bimg_decode",
        "bimg",
        "bx",
        "bae"
    }

    configuration {"vs*", "x32 or x64"}
    linkoptions {
        "/ignore:4199" -- LNK4199: /DELAYLOAD:*.dll ignored; no imports found from *.dll
    }
    links {
        -- this is needed only for testing with GLES2/3 on Windows with VS2008
        "DelayImp"
    }

    configuration {"vs201*", "x32 or x64"}
    linkoptions {
        -- this is needed only for testing with GLES2/3 on Windows with VS201x
        '/DELAYLOAD:"libEGL.dll"',
        '/DELAYLOAD:"libGLESv2.dll"'
    }

    configuration {"mingw*"}
    targetextension ".exe"
    links {
        "gdi32",
        "psapi"
    }

    configuration {"vs20*", "x32 or x64"}
    links {
        "gdi32",
        "psapi"
    }

    configuration {"durango"}
    links {
        "d3d11_x",
        "d3d12_x",
        "combase",
        "kernelx"
    }

    configuration {"asmjs"}
    kind "ConsoleApp"
    targetextension ".bc"

    configuration {"linux-* or freebsd", "not linux-steamlink"}
    links {
        "X11",
        "GL",
        "pthread"
    }

    configuration {"rpi"}
    links {
        "X11",
        "brcmGLESv2",
        "brcmEGL",
        "bcm_host",
        "vcos",
        "vchiq_arm",
        "pthread"
    }

    configuration {"osx"}
    linkoptions {
        "-framework Cocoa",
        "-framework QuartzCore",
        "-framework OpenGL",
        "-weak_framework Metal"
    }

    configuration {"ios* or tvos*"}
    kind "ConsoleApp"
    linkoptions {
        "-framework CoreFoundation",
        "-framework Foundation",
        "-framework OpenGLES",
        "-framework UIKit",
        "-framework QuartzCore",
        "-weak_framework Metal"
    }

    configuration {"xcode4", "ios"}
    kind "WindowedApp"
    files {
        path.join(BGFX_DIR, "examples/runtime/iOS-Info.plist")
    }

    configuration {"xcode4", "tvos"}
    kind "WindowedApp"
    files {
        path.join(BGFX_DIR, "examples/runtime/tvOS-Info.plist")
    }

    configuration {"qnx*"}
    targetextension ""
    links {
        "EGL",
        "GLESv2"
    }

    configuration {}

    strip()
end

function exampleProject(_combined, ...)
    if _combined then
        project("examples")
        uuid(os.uuid("examples"))
        kind "WindowedApp"

        for _, name in ipairs({...}) do
            files {
                path.join(EXAMPLES_DIR, name, "**.c"),
                path.join(EXAMPLES_DIR, name, "**.cpp"),
                path.join(EXAMPLES_DIR, name, "**.h")
            }

            removefiles {
                path.join(EXAMPLES_DIR, name, "**.bin.h")
            }
        end

        files {
            path.join(BGFX_DIR, "examples/25-c99/helloworld.c") -- hack for _main_
        }

        exampleProjectDefaults()
    else
        for _, name in ipairs({...}) do
            project("example-" .. name)
            uuid(os.uuid("example-" .. name))
            kind "WindowedApp"

            files {
                path.join(EXAMPLES_DIR, name, "**.c"),
                path.join(EXAMPLES_DIR, name, "**.cpp"),
                path.join(EXAMPLES_DIR, name, "**.h")
            }

            removefiles {
                path.join(EXAMPLES_DIR, name, "**.bin.h")
            }

            defines {
                "ENTRY_CONFIG_IMPLEMENT_MAIN=1"
            }

            exampleProjectDefaults()
        end
    end
end

dofile(BGFX_SCRIPTS_DIR .. "bgfx.lua")

group "libs"
bgfxProject("", "StaticLib", {})
dofile(path.join(BX_DIR, "scripts/bx.lua"))
dofile(path.join(BIMG_DIR, "scripts/bimg.lua"))
dofile(path.join(BIMG_DIR, "scripts/bimg_decode.lua"))
dofile("bae.lua")

group "examples"
dofile(path.join(BGFX_SCRIPTS_DIR, "example-common.lua"))
exampleProject(
    _OPTIONS["with-combined-examples"],
    "01-tonemapping",
    "02-forward-rendering",
    "03-deferred-rendering",
    "04-pbr-irb"
)

group "tools"
dofile(path.join(BGFX_SCRIPTS_DIR, "shaderc.lua"))
dofile(path.join(BGFX_SCRIPTS_DIR, "texturec.lua"))
dofile(path.join(BGFX_SCRIPTS_DIR, "texturev.lua"))
dofile(path.join(BGFX_SCRIPTS_DIR, "geometryc.lua"))
