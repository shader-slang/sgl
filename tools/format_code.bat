@echo off

set DIR=%~dp0
set PROJECT_DIR=%DIR%..\
set PYTHON=%PROJECT_DIR%tools\python\python.exe
set CLANG_FORMAT=%PROJECT_DIR%tools\clang-format\clang-format.exe

pushd %PROJECT_DIR%
call %PYTHON% %DIR%run_clang_format.py --clang-format-executable=%CLANG_FORMAT% -i -r src
popd
