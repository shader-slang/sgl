#!/bin/sh

DIR=$(dirname "$0")
PROJECT_DIR=$DIR/..
PYTHON_DIR=$PROJECT_DIR/tools/python
PYTHON=$PYTHON_DIR/python3
CLANG_FORMAT=clang-format

pushd $PROJECT_DIR
env LD_LIBRARY_PATH="$PYTHON_DIR/lib" $PYTHON $DIR/run_clang_format.py --clang-format-executable=$CLANG_FORMAT -i -r src
popd
