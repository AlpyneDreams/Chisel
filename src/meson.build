fs = import('fs')

chisel_args = windows ? [] : [
    '-Wno-shadow',
    '-Wno-volatile' # for GLM
]

chisel_src = files(
    'console/ConsoleCommands.cpp',
    'assets/loaders/Textures.cpp',
    'assets/loaders/MeshOBJ.cpp',
    windows ?
        'platform/win32/PlatformWin32.cpp' :
        'platform/linux/PlatformLinux.cpp',
    'platform/sdl/WindowSDL.cpp',
    'platform/sdl/CursorSDL.cpp',
    'render/bgfx/RenderBGFX.cpp',

    'gui/Common.cpp',
    'gui/impl/imgui_impl_sdl.cpp',
    'gui/impl/imgui_impl_bgfx.cpp',

    'chisel/Tools.cpp',
    'chisel/Chisel.cpp',
    'chisel/VMF.cpp',
    'chisel/Solid.cpp',

    'chisel/CSG/Brush.cpp',
    'chisel/CSG/CSGTree.cpp',
    'chisel/CSG/FaceCache.cpp',
    'chisel/CSG/Face.cpp',
    'chisel/CSG/Fragment.cpp',
    'chisel/CSG/Vertex.cpp',
)

chisel_deps = [
    sdl_dep,
    bgfx_dep,

    fmt_dep,
    glm_dep,
    imgui_dep,
    imguizmo_dep,
    glaze_dep,
]

chisel = executable('chisel', chisel_src,
    dependencies    : chisel_deps,
    install         : true,
    include_directories: include_directories('../submodules'),
    install_dir     : join_paths(meson.project_source_root(), 'runtime'),
    win_subsystem   : 'console',
    cpp_args        : chisel_args
)
