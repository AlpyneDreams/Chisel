@echo off
for %%F in (vs_*.glsl *.vs.glsl) do (
    echo %%F
    ..\tools\bgfx\shaderc -f %%F -o ..\runtime\core\shaders\spirv\%%~nF.bin --type v --platform linux -p spirv
)

for %%F in (fs_*.glsl *.fs.glsl) do (
    echo %%F
    ..\tools\bgfx\shaderc -f %%F -o ..\runtime\core\shaders\spirv\%%~nF.bin --type f --platform linux -p spirv
)
pause
