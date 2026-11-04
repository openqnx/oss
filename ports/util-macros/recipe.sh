# X.Org util-macros: xorg-macros.m4, the autoconf macros every X.Org module's
# configure.ac uses (XORG_DEFAULT_OPTIONS, XORG_RELEASE_VERSION,
# XORG_WITH_*, ...), and xorg-macros.pc. Data only. Version as in QNX's own
# package list.
VERSION=1.20.2
TARBALL=https://www.x.org/releases/individual/util/util-macros-1.20.2.tar.xz
SHA256=9ac269eba24f672d7d7b3574e4be5f333d13f04a7712303b1821b2a51ac82e8e
DEPS=""
# The macros run on whatever machine runs autoconf: on a target, with the
# autoconf/automake ports (aclocal finds share/aclocal/xorg-macros.m4).
BUILDSYS=autotools
