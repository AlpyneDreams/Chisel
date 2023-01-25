project('engine', 'cpp',
    version: 'latest',
    default_options: ['cpp_std=c++20', 'python.platlibdir=', 'python.purelibdir=']
)

# ImGui: Use 32-bit Wchar to support planes 1-16
add_global_arguments('-DIMGUI_USE_WCHAR32', language: 'cpp')

windows    = build_machine.system() == 'windows'

if windows
    add_global_arguments('/std:c++20', language: 'cpp')
endif

rain       = subproject('rain')
rain_dep   = rain.get_variable('rain_dep')

# Force use local WrapDB SDL2 version on Windows
sdl_dep    = dependency(windows ? 'libsdl2' : 'sdl2', fallback: 'sdl2', default_options: ['with_main=false'], static: true)

fmt_dep    = dependency('fmt', static: true)
vulkan_dep = dependency('vulkan')
vookoo_dep = declare_dependency(include_directories: 'submodules/vookoo/include')
glm_dep    = dependency('glm', static: true)
imgui_dep  = dependency('imgui', default_options: ['vulkan=disabled'], static: true)
imguizmo_dep  = dependency('imguizmo')

bgfx_dep = declare_dependency(
    sources: files(
        'submodules/bgfx/src/amalgamated.cpp',
        'submodules/bx/src/amalgamated.cpp',
        'submodules/bimg/src/image.cpp',
        'submodules/bimg/src/image_gnf.cpp',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/astc_file.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/codec.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/endpoint_codec.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/footprint.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/integer_sequence_codec.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/intermediate_astc_block.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/logical_astc_block.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/partition.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/physical_astc_block.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/quantization.cc',
        'submodules/bimg/3rdparty/astc-codec/src/decoder/weight_infill.cc',
    ),
    include_directories: include_directories([
        'submodules/bgfx/include',
        'submodules/bgfx/3rdparty',
        'submodules/bgfx/3rdparty/khronos',
        'submodules/bx/include',
        'submodules/bx/3rdparty',
        'submodules/bimg/include',
        'submodules/bimg/3rdparty/astc-codec',
        'submodules/bimg/3rdparty/astc-codec/include',
    ] + (windows ? [
        'submodules/bx/include/compat/msvc'
    ] : [])),
    compile_args: [
        '-DBX_CONFIG_DEBUG=0',
        '-DBGFX_CONFIG_RENDERER_OPENGL=0',
        '-DBGFX_CONFIG_RENDERER_VULKAN=1',
    ] + (windows ? [
        '-DPRIxPTR="llx"', # This should not be needed
        '/Zc:__cplusplus',
    ] : [])
)

subdir('src')
