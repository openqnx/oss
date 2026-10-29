VERSION=1.17.0; GIT=https://gitlab.freedesktop.org/xorg/lib/libxcb.git; REF=libxcb-1.17.0
DEPS="xcb-proto libxau libxdmcp xtrans pthread-stubs"
BUILDSYS=autotools
# libxcb's generator is a build-machine Python script that imports the xcbgen
# module xcb-proto staged; point PYTHONPATH at it.
CONF_ENV=( PYTHON=python3
           PYTHONPATH="$STAGE$PREFIX/lib/python3/site-packages:$STAGE$PREFIX/lib/python3.11/site-packages" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --without-doxygen
                  --enable-xinput --enable-xkb )
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
