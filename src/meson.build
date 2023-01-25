fs = import('fs')
python = import('python').find_installation('python3')

main = 'main/Main.cpp'

engine_src = files(
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
)

engine_args = windows ? [

] : [
    '-Wno-shadow',
    '-Wno-volatile' # for GLM
]

tools_common_src = files(
    'editor/Tools.cpp'
)

editor_src = tools_common_src + files(
    'editor/Editor.cpp'
)

hammer_src = tools_common_src + files(
    'hammer/Hammer.cpp',
    'hammer/VMF.cpp',
    'hammer/Solid.cpp',
)

# Build meta files
engine_meta = []
editor_meta = []
foreach file : engine_src + editor_src
    target = custom_target(
        fs.name(file) + '.meta.json',
        output: fs.name(file) + '.meta.json',
        input: [rain.get_variable('rain_update_py'), file],
        command: [python, '@INPUT@', '@OUTPUT@'],
        console: true
    )
    if editor_src.contains(file)
        editor_meta += [target]
    endif
    if engine_src.contains(file)
        engine_meta += [target]
    endif
endforeach

editor_rtti = custom_target(
    'rtti.cpp',
    output: 'rtti.cpp',
    input: [rain.get_variable('rain_rtti_py'), engine_meta, editor_meta],
    command: [python, '@INPUT@', '@OUTPUT@', '@CURRENT_SOURCE_DIR@'],
    console: true
)

# Deps which can be only built in engine_lib
engine_deps = [
    sdl_dep,
    bgfx_dep,
]

shared_deps = [
    rain_dep,
    fmt_dep,
    #vulkan_dep,
    #vookoo_dep,
    glm_dep,
    imgui_dep,
    imguizmo_dep,
    entt_dep
]

engine_lib = static_library('engine', engine_src,
    dependencies    : engine_deps + shared_deps,
    include_directories: include_directories('../submodules'),
    cpp_args        : engine_args
)

engine = executable('engine', main,
    dependencies  : shared_deps,
    link_with     : [engine_lib],
    install       : true,
    install_dir   : join_paths(meson.project_source_root(), 'runtime'),
    win_subsystem : 'console',
    cpp_args      : engine_args
)

editor = executable('editor', editor_src, main, editor_rtti,
    dependencies    : shared_deps,
    link_with       : [engine_lib],
    install         : true,
    install_dir     : join_paths(meson.project_source_root(), 'runtime'),
    win_subsystem   : 'console',
    cpp_args        : engine_args + ['-DEDITOR=1']
)

# TODO: Link editor_src as a lib
hammer = executable('hammer', hammer_src,
    dependencies    : shared_deps,
    link_with       : [engine_lib],
    install         : true,
    install_dir     : join_paths(meson.project_source_root(), 'runtime'),
    win_subsystem   : 'console',
    cpp_args        : engine_args + ['-DEDITOR=1']
)
