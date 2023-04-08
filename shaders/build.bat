@echo off
for %%F in (*.hlsl) do (
    echo %%F
    fxc /nologo /T vs_5_0 /E vs_main /Fo "../runtime/core/shaders/%%~nF.vsc" "%%F" > nul
    fxc /nologo /T ps_5_0 /E ps_main /Fo "../runtime/core/shaders/%%~nF.psc" "%%F" > nul
)
pause
