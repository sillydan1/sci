#!/bin/sh
set -e
echo ">>> checking if required environment is set..."
test -n "$DOCKER_TOKEN"

echo ">>> compiling..."
make
export VERSION=$(./out/bin/sci -V)

echo ">>> building source dist..."
make dist
SRC_SHA256=$(sha256sum "sci-${VERSION}.tar.gz" | awk '{ print $1 }')
sed "s/SRC_SHA256/${SRC_SHA256}/g" < PKGBUILD.in > PKGBUILD

# arch
echo ">>> building archbuilder image..."
docker build -t archbuilder -f arch-builder.dockerfile .

echo ">>> building arch package in archbuilder docker image..."
docker run --rm -it -v .:/src -e VERSION archbuilder sh -c '\
    cd && \
    cp /src/sci-$VERSION.tar.gz /src/PKGBUILD . && \
    makepkg && \
    cp *.zst /src
'

# debian
echo ">>> building debbuilder image..."
docker build -t debbuilder -f deb-builder.dockerfile .

echo ">>> building .deb in debbuilder docker image..."
docker run --rm -it -v .:/src -e VERSION debbuilder sh -c '\
    cd && \
    mkdir -p artifacts && \
    cp /src/sci-$VERSION.tar.gz . && \
    mv sci-$VERSION.tar.gz sci_$VERSION.orig.tar.gz && \
    tar xf sci_$VERSION.orig.tar.gz && \
    cd sci-$VERSION && \
    cp -r /src/debian . && \
    debuild && \
    cp ../*.deb ~/artifacts && \
    cp ../*.dsc ~/artifacts && \
    cp ../*.build ~/artifacts && \
    cp ../*.buildinfo ~/artifacts && \
    cp ../*.changes ~/artifacts && \
    cp ../*.tar.xz ~/artifacts && \
    cp ../*.tar.gz ~/artifacts && \
    cd && \
    tar czf /src/artifacts.tar.gz artifacts
'

echo ">>> building sci docker image..."
export OWNER="git.gtz.dk/agj"
docker build -t ${OWNER}/sci:${VERSION} -t ${OWNER}/sci:latest -f .dockerfile .

echo ">>> pushing latest docker image..."
# TODO: user should be some sci-bot or something, not your account. This will do for now though
docker login git.gtz.dk -u agj -p "$DOCKER_TOKEN"
docker push ${OWNER}/sci:latest
