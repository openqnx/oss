# pax-utils (scanelf, lddtree, ...). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.3.8
TARBALL=https://dev.gentoo.org/~sam/distfiles/app-misc/pax-utils/pax-utils-1.3.8.tar.xz
SHA256=12a168d1aeae2626efdbde3979da88a60ca80bb38e1db565e95ee441ff1d5a0c
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dlddtree_implementation=sh -Duse_seccomp=false -Dbuild_manpages=disabled )
