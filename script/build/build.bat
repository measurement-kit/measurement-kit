@echo off

if "%VSCMD_ARG_HOST_ARCH%" == "x64" (
    set ARCH_NAME=x86_64
)
if "%VSCMD_ARG_HOST_ARCH%" == "x86" (
    set ARCH_NAME=i386
)
if "%VSCMD_ARG_HOST_ARCH%" == "" (
    echo "Not in a Visual Studio Command Prompt"
    pause
    exit 1
)

for %%I in (%*) do ubuntu.exe run ./cross-windows %ARCH_NAME% ./%%I
