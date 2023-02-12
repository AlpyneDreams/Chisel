
if [ -z "$BGFX_SHADERC" ]; then
BGFX_SHADERC="../submodules/bgfx/.build/linux64_gcc/bin/shadercRelease"
    if [ ! -f "$BGFX_SHADERC" ]; then
        BGFX_SHADERC="../tools/bgfx/shaderc"
    fi
fi

ARGS="--platform linux"

for f in *.vert; do
    echo $f
    $BGFX_SHADERC -f $f -o ../runtime/core/shaders/spirv/$f.bin --type v -p spirv $ARGS
    $BGFX_SHADERC -f $f -o ../runtime/core/shaders/glsl/$f.bin --type v -p 440 $ARGS
done

for f in *.frag; do
    echo $f
    $BGFX_SHADERC -f $f -o ../runtime/core/shaders/spirv/$f.bin --type f -p spirv $ARGS
    $BGFX_SHADERC -f $f -o ../runtime/core/shaders/glsl/$f.bin --type f -p 440 $ARGS
done

