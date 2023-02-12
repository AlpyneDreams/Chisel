project('engine', 'cpp',
    version: 'latest',
    default_options: ['cpp_std=c++20']
)

# ImGui: Use 32-bit Wchar to support planes 1-16
add_global_arguments('-DIMGUI_USE_WCHAR32', language: 'cpp')

windows    = build_machine.system() == 'windows'

# Force use local WrapDB SDL2 version on Windows
sdl_dep    = dependency(windows ? 'libsdl2' : 'sdl2', fallback: 'sdl2', default_options: ['with_main=false'], static: true)

fmt_dep    = dependency('fmt', static: true)
glm_dep    = dependency('glm', static: true)
imgui_dep  = dependency('imgui', default_options: ['vulkan=disabled'], static: true)
imguizmo_dep  = dependency('imguizmo')

cmake = import('cmake')
glaze_proj = cmake.subproject('glaze')
glaze_dep = glaze_proj.dependency('glaze_glaze')

bgfx_dep = declare_dependency(
    sources: files(
        'submodules/bgfx/src/amalgamated.cpp',
        'submodules/bx/src/amalgamated.cpp',
        'submodules/bimg/src/image.cpp',
        'submodules/bimg/src/image_gnf.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_averages_and_directions.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_block_sizes.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_color_quantize.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_color_unquantize.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_compress_symbolic.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_compute_variance.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_decompress_symbolic.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_diagnostic_trace.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_entry.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_find_best_partitioning.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_ideal_endpoints_and_weights.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_image.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_integer_sequence.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_mathlib.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_mathlib_softfloat.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_partition_tables.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_percentile_tables.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_pick_best_endpoint_format.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_platform_isa_detection.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_quantization.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_symbolic_physical.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_weight_align.cpp',
        'submodules/bimg/3rdparty/astc-encoder/source/astcenc_weight_quant_xfer_tables.cpp',
    ),
    include_directories: include_directories([
        'submodules/bgfx/include',
        'submodules/bgfx/3rdparty',
        'submodules/bgfx/3rdparty/khronos',
        'submodules/bx/include',
        'submodules/bx/3rdparty',
        'submodules/bimg/include',
        'submodules/bimg/3rdparty/astc-encoder',
        'submodules/bimg/3rdparty/astc-encoder/include',
    ] + (windows ? [
        'submodules/bx/include/compat/msvc'
    ] : [])),
    compile_args: [
        '-DBX_CONFIG_DEBUG=0',
        '-DBGFX_CONFIG_RENDERER_OPENGL=0',
        '-DBGFX_CONFIG_RENDERER_VULKAN=1',
    ] + (windows ? [
        '-DPRIxPTR="llx"', # This should not be needed
        '-DPRIx64="llx"', # This should not be needed
        '/Zc:__cplusplus',
    ] : [])
)

subdir('src')
