# libraqm: complex text layout (bidirectional text, shaping, script
# itemization) on FreeType, HarfBuzz and FriBidi -- what matplotlib 3.11
# lays text out with.
VERSION=0.11.0; GIT=https://github.com/HOST-Oman/libraqm.git; REF=v0.11.0
DEPS="freetype harfbuzz fribidi"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=false -Dtests=false -Dsheenbidi=false )
