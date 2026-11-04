# libcaca: a graphics library that outputs text instead of pixels -- ASCII/ANSI
# art from images and canvases, in a terminal (ncurses) or exported to text,
# ANSI, HTML, SVG ...; with img2txt, cacaview, cacademo, figlet-like toilet
# fonts (cacafire, cacaclock).
VERSION=0.99.beta20; GIT=https://github.com/cacalabs/libcaca.git; REF=v0.99.beta20
DEPS="ncurses zlib"
BUILDSYS=autotools
# - the ncurses terminal driver and the C++ binding; no X11/OpenGL/slang
#   drivers (no X server on QNX targets; slang is not ported), no
#   Java/Python/Ruby/C# bindings, no Imlib2 (img2txt loads BMP natively);
# - no documentation (doxygen + LaTeX) and no unit/fuzz tests.
CONF_ARGS_EXTRA=( --disable-static --enable-ncurses --enable-cxx
                  --disable-x11 --disable-gl --disable-slang --disable-cocoa
                  --disable-java --disable-python --disable-ruby --disable-csharp
                  --disable-imlib2 --disable-doc --disable-cppunit --disable-zzuf )
