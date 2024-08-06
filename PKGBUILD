# Maintainer: Asger Gitz-Johansen <asger.gitz@hotmail.com>
pkgname=sci
pkgver=1.0.0
pkgrel=1
epoch=
pkgdesc="A simple / minimal CI (Continuous Integration) system"
arch=('x86_64')  # TODO: also arm64 when you're not tired
url="gitea.local:3000/agj/sci"
license=('GPL-3.0-or-later')  # TODO: add LICENSE file and header
groups=()
depends=("glibc"
    "util-linux-libs")
makedepends=()
checkdepends=()
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=("$pkgname-$pkgver.tar.gz")
noextract=()
sha256sums=("948092bdcc3591afcdc205263832a06c838aa9d524c762b061e91cffa04b7d63")
validpgpkeys=()

build() {
	cd "$pkgname-$pkgver"
	make
}

package() {
	cd "$pkgname-$pkgver"
	make DESTDIR="$pkgdir/" PREFIX="/usr" install
}
