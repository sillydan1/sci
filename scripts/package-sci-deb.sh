#!/bin/bash
# TODO: SCI_VERSION env var
debbuilder_image=$(docker images | grep debbuilder)
set -e
if [ -z $debbuilder_image ]; then
    echo "building debbuilder image..."
    docker build -t debbuilder -f deb-builder.dockerfile .
fi

echo "building source dist..."
make dist

echo "building .deb in debbuilder docker image..."
docker run --rm -it -v .:/src debbuilder sh -c '\
    cd && \
    mkdir -p artifacts && \
    cp /src/sci-1.0.0.tar.gz . && \
    mv sci-1.0.0.tar.gz sci_1.0.0.orig.tar.gz && \
    tar xf sci_1.0.0.orig.tar.gz && \
    cd sci-1.0.0 && \
    cp -r /src/debian . && \
    debuild && \
    cp ../*.deb ~/artifacts && \
    cp ../*.dsc ~/artifacts && \
    cp ../*.build ~/artifacts && \
    cp ../*.buildinfo ~/artifacts && \
    cp ../*.changes ~/artifacts && \
    cp ../*.tar.xz ~/artifacts && \
    cp ../*.tar.gz ~/artifacts && \
    tar czf /src/artifacts.tar.gz ~/artifacts
'
