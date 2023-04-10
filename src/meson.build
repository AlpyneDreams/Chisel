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
    'render/Render.cpp',

    'gui/Common.cpp',
    'gui/Layout.cpp',
    'gui/Inspector.cpp',
    'gui/View3D.cpp',
    'gui/Viewport.cpp',
    'gui/impl/imgui_impl_sdl.cpp',
    'gui/impl/imgui_impl_dx11.cpp',

    'chisel/Tools.cpp',
    'chisel/Selection.cpp',
    'chisel/Chisel.cpp',
    'chisel/Handles.cpp',
    'chisel/MapRender.cpp',
    'chisel/FGD/FGD.cpp',
    'chisel/VMF/VMF.cpp',
    'chisel/VMF/Solid.cpp',

    'chisel/CSG/Brush.cpp',
    'chisel/CSG/CSGTree.cpp',
    'chisel/CSG/FaceCache.cpp',
    'chisel/CSG/Face.cpp',
    'chisel/CSG/Fragment.cpp',
    'chisel/CSG/Vertex.cpp',
)

chisel_deps = [
    sdl_dep,
    d3d11_dep,
    dxgi_dep,

    fmt_dep,
    glm_dep,
    imgui_dep,
    imguizmo_dep,
    glaze_dep,
]

chisel = executable('chisel', chisel_src, offsetallocator_src,
    dependencies    : chisel_deps,
    include_directories: include_directories('../submodules'),
    win_subsystem   : 'console',
    cpp_args        : chisel_args
)

rm = windows ? 'del' : 'rm'
copy = windows ? 'copy' : ['cp', '-f']

if fs.exists(chisel.full_path())
    exe_path = join_paths(meson.project_source_root(), 'runtime', fs.stem(chisel.full_path()))
    run_command(rm, exe_path, check: false)
    run_command(copy, chisel.full_path(), exe_path, check: false)
endif
