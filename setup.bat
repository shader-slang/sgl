: This script is fetching all dependencies via packman.

@echo off
setlocal

set VCPKG=%~dp0\external\vcpkg\vcpkg.exe
set VCPKG_TRIPLET=x64-windows
set VCPKG_MANIFEST_ROOT=%~dp0
set VCPKG_INSTALL_ROOT=%~dp0\external\installed
set PACKMAN=%~dp0\tools\packman\packman.cmd
set PLATFORM=windows-x86_64

echo Installing dependencies using vcpkg ...

cmd.exe /C %~dp0\external\vcpkg\bootstrap-vcpkg.bat -disableMetrics
cmd.exe /C %VCPKG% install --triplet=%VCPKG_TRIPLET% --x-manifest-root=%VCPKG_MANIFEST_ROOT% --x-install-root=%VCPKG_INSTALL_ROOT%
: --overlay-ports=overlay-ports --overlay-triplets=overlay-triplets

echo Updating git submodules ...

where /q git
if errorlevel 1 (
    echo Cannot find git on PATH! Please initialize submodules manually and rerun.
    exit /b 1
) ELSE (
    git submodule sync --recursive
    git submodule update --init --recursive
)

echo Fetching dependencies ...

call %PACKMAN% pull --platform %PLATFORM% %~dp0\dependencies.xml
if errorlevel 1 goto error

if not exist %~dp0\.vscode\ (
    echo Setting up VS Code workspace ...
    xcopy %~dp0\.vscode-default\ %~dp0\.vscode\ /y
)

exit /b 0

:error
echo Failed to fetch dependencies!
exit /b 1
