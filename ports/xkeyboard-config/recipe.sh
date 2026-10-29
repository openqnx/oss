VERSION=2.43
GIT=https://gitlab.freedesktop.org/xkeyboard-config/xkeyboard-config.git; REF=xkeyboard-config-2.43
BUILDSYS=meson
# Pure data: the XKB rules, symbols and geometry files libxkbcommon reads at
# runtime. Nothing is compiled, so the cross file only decides where it lands.
MESON_ARGS_EXTRA=( -Dxorg-rules-symlinks=true -Dcompat-rules=true )
