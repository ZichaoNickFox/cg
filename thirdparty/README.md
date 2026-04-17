# Third-Party Dependencies

This repository vendors its third-party dependencies under `thirdparty/`.

## Initialize

```bash
git submodule update --init --recursive
```

## Layout

- `assimp`, `eigen`, `fmt`, `glad`, `glfw`, `glm`, `glog`, `googletest`, `imgui`,
  `implot`, `ordered-map`, `protobuf`, and `stb` are pinned as git submodules.
- `generated/glad/` contains the checked-in OpenGL 4.5 loader generated from the
  vendored `glad` source. The normal build uses these generated sources directly,
  so local builds do not depend on Python packages or network access.

## Notes

- The project no longer uses CMake `FetchContent` to download dependencies at
  configure time.
- If a submodule is missing, CMake will stop with an explicit error that tells
  you to initialize submodules first.
