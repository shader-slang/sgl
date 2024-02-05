@echo off

set DIR=%~dp0
set PROJECT_DIR=%DIR%..\

pushd %PROJECT_DIR%
call python.exe %DIR%fix_comments.py -r src
popd
