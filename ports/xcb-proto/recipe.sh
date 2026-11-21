VERSION=1.17.0; GIT=https://gitlab.freedesktop.org/xorg/proto/xcbproto.git; REF=xcb-proto-1.17.0
# Pure XML + a Python module consumed by libxcb's generator at build time. It
# installs no target code, so it is built with the host Python.
BUILDSYS=autotools
CONF_ARGS_EXTRA=( )
# xcbgen goes where the target's Python (the python3 port, 3.14) looks, not
# under the build host's Python version (3.12 in the clean build image:
# lib/python3.12, which the target never searches).
CONF_ENV=( am_cv_python_pythondir='${prefix}/lib/python3.14/site-packages' )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
