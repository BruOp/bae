BAE_SRC_DIR = path.join(BAE_DIR, "src")
BAE_INCLUDE_DIR = path.join(BAE_DIR, "include", "bae")

TINYGLTF_DIR = path.join(EXTERNAL_DIR, "tinygltf/")
MIKKTSPACE_DIR = path.join(EXTERNAL_DIR, "mikktspace")

project("bae")
uuid(os.uuid("bae"))
kind "StaticLib"

files {
  path.join(BAE_SRC_DIR, "**.cpp"),
  path.join(BAE_INCLUDE_DIR, "**.h"),
  path.join(MIKKTSPACE_DIR, "mikktspace.c")
}

removefiles {
  path.join(BAE_DIR, "**.bin.h")
}

includedirs {
  BAE_INCLUDE_DIR,
  path.join(BX_DIR, "include"),
  path.join(BIMG_DIR, "include"),
  path.join(BGFX_DIR, "include"),
  path.join(BGFX_DIR, "examples/common"),
  path.join(GLM_DIR, "include"),
  path.join(TINYGLTF_DIR, "include"),
  path.join(MIKKTSPACE_DIR, "include"),
}

links {
  "example-common",
  "bgfx",
  "bimg_decode",
  "bimg",
  "bx"
}

configuration {}

strip()
