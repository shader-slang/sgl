@echo off

set pwd=%~dp0
set project_dir=%pwd%..\
set python=%project_dir%tools\python\python.exe
set clang_format=%project_dir%tools\clang-format\clang-format.exe

pushd %project_dir%
call %python% %pwd%run_clang_format.py --clang-format-executable=%clang_format% -i -r src
popd
