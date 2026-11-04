# mate-common: the autoconf macros (MATE_COMPILE_WARNINGS, MATE_DEBUG_CHECK,
# MATE_MAINTAINER_MODE_DEFINES, ...) and the mate-autogen / mate-doc-common
# scripts that MATE desktop modules bootstrap with. Data and shell scripts.
# Version as in QNX's own package list.
VERSION=1.28.0
TARBALL=https://pub.mate-desktop.org/releases/1.28/mate-common-1.28.0.tar.xz
SHA256=42b7c2cee268f71d7e1c1ad4f69bcda0ecd65578a96722187c6b76378d26ba0a
DEPS=""
# mate-autogen runs autoreconf, aclocal and automake: on a target, the
# autoconf and automake ports.
RUNTIME_DEPS="autoconf automake"
BUILDSYS=autotools
