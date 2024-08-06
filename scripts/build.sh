#!/bin/sh
set -e # exit immediately on error

git clone -b dev "$SCI_PIPELINE_URL" "$SCI_PIPELINE_NAME"
echo "clone success"

cd "$SCI_PIPELINE_NAME"

cmake -B build
echo "configure success"

cmake --build build
echo "build success"

cpack build
echo "packaging success"

# TODO: upload artifacts to some artifact store
# curl "build/dist/your-package.deb" > ftp://example.com/artifacts
# echo "artifacts upload success"
