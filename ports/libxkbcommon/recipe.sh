VERSION=1.8.1; GIT=https://github.com/xkbcommon/libxkbcommon.git; REF=xkbcommon-1.8.1
DEPS="libxml2"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Denable-docs=false -Denable-wayland=false -Denable-x11=false
                   -Denable-xkbregistry=true -Denable-tools=false -Denable-bash-completion=false
                   -Dxkb-config-root=/usr/local/share/X11/xkb -Dx-locale-root=/usr/local/share/X11/locale )
# Where the keymaps are on the target. Since 1.8 the default comes from
# xkeyboard-config's pkg-config file -- with the sysroot in front, the build
# machine's staging path, compiled in: every keymap failed to load
# ("failed to add default include path /home/.../stage/...").
