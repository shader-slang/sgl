#!/bin/sh

pwd=$(dirname "$0")
project_dir=$pwd/..
python_dir=$project_dir/tools/python
python=$python_dir/python3
clang_format=clang-format

pushd $project_dir
env LD_LIBRARY_PATH="$python_dir/lib" $python $pwd/run_clang_format.py --clang-format-executable=$clang_format -i -r src
popd
