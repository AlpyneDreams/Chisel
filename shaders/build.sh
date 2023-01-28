
if [ -z "$BGFX_SHADERC" ]; then
BGFX_SHADERC="../submodules/bgfx/.build/linux64_gcc/bin/shadercRelease"
    if [ ! -f "$BGFX_SHADERC" ]; then
        BGFX_SHADERC="../tools/bgfx/shaderc"
    fi
fi

ARGS="--platform linux -p spirv"

for f in *.vert; do
    echo $f
    $BGFX_SHADERC -f $f -o ../runtime/core/shaders/spirv/$f.bin --type v $ARGS
done

for f in *.frag; do
    echo $f
    $BGFX_SHADERC -f $f -o ../runtime/core/shaders/spirv/$f.bin --type f $ARGS
done

