fs = import('fs')

chisel_args = windows ? [] : [
    '-Wno-shadow',
    '-Wno-volatile' # for GLM
]

chisel_src = [
    'console/ConsoleCommands.cpp',
    'assets/Assets.cpp',
    'assets/loaders/Textures.cpp',
    'assets/loaders/Materials.cpp',
    'assets/loaders/MeshOBJ.cpp',
    windows ?
        'platform/win32/PlatformWin32.cpp' :
        'platform/linux/PlatformLinux.cpp',
    'platform/sdl/WindowSDL.cpp',
    'platform/sdl/CursorSDL.cpp',
    'render/Render.cpp',

    'gui/Common.cpp',
    'gui/Layout.cpp',
    'gui/AssetPicker.cpp',
    'gui/Inspector.cpp',
    'gui/View3D.cpp',
    'gui/Viewport.cpp',
    'gui/impl/imgui_impl_sdl.cpp',
    'gui/impl/imgui_impl_dx11.cpp',

    'chisel/Engine.cpp',
    'chisel/Selection.cpp',
    'chisel/Chisel.cpp',
    'chisel/Handles.cpp',
    'chisel/Gizmos.cpp',
    'chisel/MapRender.cpp',
    'chisel/tools/Tool.cpp',
    'chisel/tools/BlockTool.cpp',
    'chisel/tools/ClipTool.cpp',
    'chisel/tools/EntityTool.cpp',
    'chisel/tools/PlacementTool.cpp',
    'chisel/tools/SelectTool.cpp',
    'chisel/tools/TransformTool.cpp',
    'chisel/FGD/FGD.cpp',
    'chisel/map/Face.cpp',
    'chisel/map/Solid.cpp',
    'chisel/map/Entity.cpp',
    'chisel/map/Map.cpp',
    
    'chisel/formats/FormatVMF.cpp',
    'chisel/formats/FormatMap.cpp',
    'chisel/formats/FormatBox.cpp',
]

chisel_link_args = []

if windows
    chisel_args += '-D_WIN32_WINNT=0xa00'
    chisel_link_args = [
        '/MANIFEST:EMBED',
        '/MANIFESTINPUT:' + meson.source_root() + '/src/platform/win32/windows_default.manifest',
        '/MANIFESTINPUT:' + meson.source_root() + '/src/platform/win32/windows_dpiaware.manifest',
        '/MANIFESTINPUT:' + meson.source_root() + '/src/platform/win32/windows_visualstyles.manifest',
    ]
endif

chisel_deps = [
    sdl_dep,
    d3d11_dep,
    dxgi_dep,

    fmt_dep,
    glm_dep,
    imgui_dep,
    imguizmo_dep,
    zstd_dep,
]

chisel = executable('chisel', chisel_src, offsetallocator_src, yyjson_src,
    dependencies    : chisel_deps,
    include_directories: include_directories('../submodules'),
    win_subsystem   : 'console',
    cpp_args        : chisel_args,
    link_args       : chisel_link_args,
)

copy = windows ? ['powershell', 'cp'] : ['cp', '-f']

custom_target('copy_exe',
    command: [copy,
        chisel.full_path(),
        join_paths(meson.project_source_root(), 'runtime', fs.name(chisel.full_path()))
    ],
    output: 'fake',
    depends: chisel,
    build_by_default: true
)
