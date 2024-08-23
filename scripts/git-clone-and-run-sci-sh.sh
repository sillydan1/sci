#!/bin/sh
set -e
echo ">>> cloning..."
git clone $SCI_PIPELINE_URL $SCI_PIPELINE_NAME
cd $SCI_PIPELINE_NAME

echo ">>> running .sci.sh..."
time sh .sci.sh
