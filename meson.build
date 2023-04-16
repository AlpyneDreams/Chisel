project('chisel', ['c', 'cpp'],
    version: 'latest',
    default_options: ['cpp_std=c++20']
)

cpp = meson.get_compiler('cpp')

# ImGui: Use 32-bit Wchar to support planes 1-16
add_global_arguments('-DIMGUI_USE_WCHAR32', language: 'cpp')

windows    = build_machine.system() == 'windows'
linux      = build_machine.system() == 'linux'

sdl_dep    = dependency('sdl2', default_options: ['with_main=false'], static: true)

fmt_dep    = dependency('fmt', static: true)
glm_dep    = dependency('glm', static: true)
imgui_dep  = dependency('imgui', default_options: ['vulkan=disabled'], static: true)
imguizmo_dep  = dependency('imguizmo')

dxvk_opts = [
    'enable_d3d9=false',
    'enable_d3d10=false',
]

d3d11_dep = windows ? cpp.find_library('d3d11') : dependency('d3d11', default_options: dxvk_opts)
dxgi_dep = windows ? cpp.find_library('dxgi') : dependency('dxgi', default_options: dxvk_opts)

offsetallocator_src = files('submodules/OffsetAllocator/offsetAllocator.cpp')
yyjson_src = files('submodules/yyjson/src/yyjson.c')

subdir('src')
