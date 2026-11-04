# gumbo-parser: an HTML5 parsing library in pure C99 (the WHATWG parsing
# algorithm), originally Google's, maintained on Codeberg. litehtml parses
# its documents with it.
VERSION=0.13.2; GIT=https://codeberg.org/grisha/gumbo-parser.git; REF=0.13.2
DEPS=""
BUILDSYS=meson
# The shared library only: its tests need gtest, the examples are demos.
MESON_ARGS_EXTRA=( -Dtests=false -Dexamples=false -Dfuzz=false )
