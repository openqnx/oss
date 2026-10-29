VERSION=1.17.0; GIT=https://gitlab.freedesktop.org/xorg/proto/xcbproto.git; REF=xcb-proto-1.17.0
# Pure XML + a Python module consumed by libxcb's generator at build time. It
# installs no target code, so it is built with the host Python.
BUILDSYS=autotools
CONF_ARGS_EXTRA=( )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
