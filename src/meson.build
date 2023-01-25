fs = import('fs')
python = import('python').find_installation('python3')

engine_args = windows ? [] : [
    '-Wno-shadow',
    '-Wno-volatile' # for GLM
]

hammer_src = files(
    # engine_src
    'assets/loaders/Textures.cpp',
    'assets/loaders/MeshOBJ.cpp',
    windows ?
        'platform/win32/PlatformWin32.cpp' :
        'platform/linux/PlatformLinux.cpp',
    'platform/sdl/WindowSDL.cpp',
    'platform/sdl/CursorSDL.cpp',
    'render/bgfx/RenderBGFX.cpp',
    #'render/vk/RenderVK.cpp',
    'render/pipelines/Forward.cpp',

    'imgui/Common.cpp',
    'imgui/impl/imgui_impl_sdl.cpp',
    'imgui/impl/imgui_impl_bgfx.cpp',

    'editor/Tools.cpp',
    'hammer/Hammer.cpp',
    'hammer/VMF.cpp',
    'hammer/Solid.cpp',
)

hammer_deps = [
    sdl_dep,
    bgfx_dep,
    
    rain_dep,
    fmt_dep,
    glm_dep,
    imgui_dep,
    imguizmo_dep
]

# TODO: Link editor_src as a lib
hammer = executable('hammer', hammer_src,
    dependencies    : hammer_deps,
    install         : true,
    include_directories: include_directories('../submodules'),
    install_dir     : join_paths(meson.project_source_root(), 'runtime'),
    win_subsystem   : 'console',
    cpp_args        : engine_args + ['-DEDITOR=1']
)
