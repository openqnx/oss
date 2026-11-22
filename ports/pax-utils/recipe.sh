# pax-utils (scanelf, lddtree, ...). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.3.9
TARBALL=https://dev.gentoo.org/~sam/distfiles/app-misc/pax-utils/pax-utils-1.3.9.tar.xz
SHA256=7409f246b3c59b3e2d459d2232a6f7374469637ae6cf616c115b8d5830cab840
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlddtree_implementation=sh -Duse_seccomp=false -Dbuild_manpages=disabled )
