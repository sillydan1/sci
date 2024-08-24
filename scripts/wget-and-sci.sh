#!/bin/sh
# NOTE: This script assumes that the url is a .tar.gz file.
# TODO: check if $# is >= 1 and give a warning that the extract dir should be provided.
set -ex  # print all that we're doing (no need for echo's)
tmpdir=$(mktemp -d)
wget "$SCI_PIPELINE_URL" -P "$tmpdir"
cd "$tmpdir"
tar xf *.tar.gz
cd $1
sh .sci.sh
cd -
rm -rf "$tmpdir"
