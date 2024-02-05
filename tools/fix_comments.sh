#!/bin/bash

DIR=$(dirname "$0")
PROJECT_DIR=${DIR}/..

pushd ${PROJECT_DIR} > /dev/null
python ${DIR}/fix_comments.py -r src
popd > /dev/null
