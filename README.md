# ENGINE

<img width="961" alt="editor_4LhbOctQWX" src="https://user-images.githubusercontent.com/3376691/179876459-36eae6dc-5d7c-43b9-a834-0d9fc675294b.png">

This is engineering. Most of this code was written under the influence, so do not read too much into it. This codebase is not intended to have an orthodox structure or coherency. There are many branches with significant changes.

## Dependencies ##

- [Meson](https://mesonbuild.com/) is required to build.
  - You may need [CMake](https://cmake.org/) to build EnTT
- To generate RTTI information, you will need:
  - [Python 3](https://www.python.org/)
  - The `libclang` pip package

Handled by Meson (via pkgconfig or WrapDB):
- [fmt](https://fmt.dev/)
- [SDL](https://www.libsdl.org/)
- [GLM](https://github.com/g-truc/glm)
- [Dear ImGui](https://github.com/ocornut/imgui)
- [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) (for now)
- [EnTT](https://github.com/skypjack/entt) (for now)
- ~~[Vulkan](https://www.vulkan.org/)~~ (currently not used)

Included as Git submodules:
- [rain](https://github.com/AlpyneDreams/rain) - RTTI library
- [bgfx](https://github.com/bkaradzic/bgfx) (with [bx](https://github.com/bkaradzic/bx) and [bimg](https://github.com/bkaradzic/bimg))
- ~~[Vookoo](https://github.com/andy-thomason/Vookoo)~~ (currently not used)

Already included in repo:
- [tinyobjloader](https://github.com/tinyobjloader/tinyobjloader)
- [stb](https://github.com/nothings/stb) image loader and writer
- [imgui_impl_bgfx.cpp](https://gist.github.com/RichardGale/6e2b74bc42b3005e08397236e4be0fd0)
- [Material Design Icons Font](https://materialdesignicons.com/)

## Building ##

After installing all dependencies, use the tasks in `.vscode/tasks.json` to quickly setup and build. 
Or use the following commands to 

To setup (run once):
```
meson build
```

To build:
```
meson install -C build
```

