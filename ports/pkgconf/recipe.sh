VERSION=2.3.0
# The release tarball: pkgconf's git tree needs autoreconf and the tarball
# ships a generated configure.  It also carries its own meson.build, which is
# the simpler path here.
TARBALL=https://distfiles.ariadne.space/pkgconf/pkgconf-2.3.0.tar.xz
SHA256=3a9080ac51d03615e7c1910a0a2a8df08424892b5f13b0628a204d3fcce0ea8b
BUILDSYS=meson
# A *target* pkg-config, for software built on the target itself.  The cross
# build on this machine uses hosttools/bin/pkg-config-native and never this
# binary -- so nothing in the workspace depends on it, and it is here because
# a QNX image that carries development headers wants one.
MESON_ARGS_EXTRA=( -Dtests=disabled )
