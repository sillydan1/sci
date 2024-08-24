#!/bin/sh
set -ex  # print all that we're doing (no need for echo's)
tmpdir=$(mktemp -d)
git clone --depth=1 --recurse-submodules --shallow-submodules -b $1 "$SCI_PIPELINE_URL" "$tmpdir"
shift
cd "$tmpdir"
sh .sci.sh
cd -
