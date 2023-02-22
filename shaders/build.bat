@echo off
for %%F in (*.vert) do (
    echo %%F
    ..\tools\bgfx\shaderc -f %%F -o ..\runtime\core\shaders\dx11\%%F.bin --type v --platform windows -p s_5_0
)

for %%F in (*.frag) do (
    echo %%F
    ..\tools\bgfx\shaderc -f %%F -o ..\runtime\core\shaders\dx11\%%F.bin --type f --platform windows -p s_5_0
)
pause
