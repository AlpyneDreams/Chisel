@echo off
for %%F in (*.vert) do (
    echo %%F
    ..\tools\bgfx\shaderc -f %%F -o ..\runtime\core\shaders\spirv\%%F.bin --type v --platform linux -p spirv
)

for %%F in (*.frag) do (
    echo %%F
    ..\tools\bgfx\shaderc -f %%F -o ..\runtime\core\shaders\spirv\%%F.bin --type f --platform linux -p spirv
)
pause
