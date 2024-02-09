# Chisel

## Dependencies ##

[Meson](https://mesonbuild.com/) is required to build.

Handled by Meson (via pkgconfig or WrapDB):
- [dxvk](https://github.com/doitsujin/dxvk)
- [fmt](https://fmt.dev/)
- [SDL](https://www.libsdl.org/)
- [GLM](https://github.com/g-truc/glm)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo)
- [zstd](https://github.com/facebook/zstd)

Included as Git submodules:
- [OffsetAllocator](https://github.com/sebbbi/OffsetAllocator)
- [libvpk-plusplus](https://github.com/Joshua-Ashton/libvpk-plusplus)
- [libvtf-plusplus](https://github.com/Joshua-Ashton/libvtf-plusplus)
- [yyjson](https://github.com/ibireme/yyjson)

Bundled in repo:
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [stb](https://github.com/nothings/stb) image loader and writer
- [Material Design Icons Font](https://materialdesignicons.com/)
- [nlohmann/json](https://github.com/nlohmann/json)

## Building ##

First, make sure you've got submodules:

```
git submodule init
git submodule update
```

After installing all dependencies, you can use the tasks in `.vscode/tasks.json` to quickly setup and build. Or run the following commands:

To setup (run once):
```
meson setup build
```

To build:
```
meson install -C build
```

