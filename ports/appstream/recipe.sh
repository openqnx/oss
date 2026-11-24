# AppStream: library and tool (appstreamcli) to read, write and validate
# AppStream software metadata (metainfo/catalog XML and YAML). Dependency of
# libadwaita, whose AdwAboutDialog/AdwAboutWindow can be filled in from an
# application's metainfo.
# Since 1.1 AppStream reads and writes YAML with libfyaml (not libyaml).
VERSION=1.2.3
TARBALL=https://www.freedesktop.org/software/appstream/releases/AppStream-1.2.3.tar.xz
SHA256=8daa01a4d66c73f036aa92af9bf76abfe8408eaa76fcda6ca54fa8ef5b93ed17
DEPS="glib curl libxml2 libfyaml libxmlb zstd wayland"
BUILDSYS=meson
# The tarball's paths start with ./AppStream-<version>/ (one level more than
# usual); the patches name that directory too.
SUBDIR=AppStream-$VERSION
# No systemd, stemming (libstemmer), Qt, GObject introspection/Vala, SVG
# icon rendering (the compose module, which has it now), BLAKE3 checksums (libblake3 is not
# ported) or documentation; -Dman=false: the build machine has no DocBook XSL
# (patches/0002 made the man page optional before 1.1 had this option).
# Display detection (wayland): only wayland-client's headers at build time,
# the library is loaded at run time when it is there.
MESON_ARGS_EXTRA=( -Dstemming=false -Dsystemd=false -Dvapi=false -Dqt=false -Dcompose=false
                   -Dapt-support=false -Dgir=false -Dzstd-support=true
                   -Dblake3-support=false -Ddisplay-detection=wayland
                   -Ddocs=false -Dapidocs=false -Dinstall-docs=false -Dman=false )
