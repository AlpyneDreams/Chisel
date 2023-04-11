@echo off

if [%DXINCLUDE%] == [] (
    call "C:\Program Files (x86)\Microsoft DirectX SDK (June 2010)\Utilities\bin\dx_setenv.cmd"
    set DXINCLUDE=1
)

for %%F in (*.hlsl) do (
    echo %%F
    fxc /nologo /T vs_5_0 /E vs_main /Fo "../runtime/core/shaders/%%~nF.vsc" "%%F" > nul
    fxc /nologo /T ps_5_0 /E ps_main /Fo "../runtime/core/shaders/%%~nF.psc" "%%F" > nul
)

for %%F in (*.compute) do (
    echo %%F
    fxc /nologo /T cs_5_0 /E cs_main /Fo "../runtime/core/shaders/%%~nF.csc" "%%F" > nul
)
