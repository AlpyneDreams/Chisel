project('chisel', 'cpp',
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

cmake = import('cmake')
glaze_proj = cmake.subproject('glaze')
glaze_dep = glaze_proj.dependency('glaze_glaze')

d3d11_dep = cpp.find_library('d3d11')
dxgi_dep = cpp.find_library('dxgi')

offsetallocator_src = files( 'submodules/OffsetAllocator/offsetAllocator.cpp' )

subdir('src')
