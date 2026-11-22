# fcft: font loading and glyph rasterization on fontconfig, FreeType and
# pixman, with HarfBuzz/utf8proc text shaping -- the font library of the foot
# terminal.
VERSION=3.3.2; GIT=https://codeberg.org/dnkl/fcft.git; REF=3.3.2
DEPS="tllist fontconfig freetype harfbuzz utf8proc pixman"
BUILDSYS=meson
# - grapheme and run shaping on (foot uses both; HarfBuzz and utf8proc are here);
# - SVG glyphs with the bundled nanosvg (upstream's default);
# - no man pages: they need scdoc on the build machine.
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dgrapheme-shaping=enabled -Drun-shaping=enabled
                   -Dsvg-backend=nanosvg -Dtest-text-shaping=false )
