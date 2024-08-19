#!/bin/bash
archbuilder_image=$(docker images | grep archbuilder)
set -e
if [ -z $archbuilder_image ]; then
    echo "building archbuilder image..."
    docker build -t archbuilder -f arch-builder.dockerfile .
fi

echo "building source dist..."
make dist
SRC_SHA256=$(sha256sum sci-1.0.0.tar.gz | awk '{ print $1 }')
echo $SRC_SHA256
sed "s/SRC_SHA256/$SRC_SHA256/g" < PKGBUILD.in > PKGBUILD

echo "building arch package in archbuilder docker image..."
docker run --rm -it -v .:/src archbuilder sh -c '\
    cd && \
    cp /src/sci-1.0.0.tar.gz /src/PKGBUILD . && \
    makepkg && \
    cp *.zst /src
'
