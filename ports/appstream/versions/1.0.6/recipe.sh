# AppStream: library and tool (appstreamcli) to read, write and validate
# AppStream software metadata (metainfo/catalog XML and YAML). Dependency of
# libadwaita, whose AdwAboutDialog/AdwAboutWindow can be filled in from an
# application's metainfo.
# The 1.0 series: 1.1 and later replace libyaml with libfyaml, which is not
# ported here, and libadwaita needs nothing newer.
VERSION=1.0.6
TARBALL=https://www.freedesktop.org/software/appstream/releases/AppStream-1.0.6.tar.xz
SHA256=db4439db6a33de3ca1041473501610844ddf1b72ae23016c05242c681c380b4d
DEPS="glib curl libxml2 libyaml libxmlb zstd"
BUILDSYS=meson
# The tarball's paths start with ./AppStream-1.0.6/ (one level more than usual).
SUBDIR=AppStream-1.0.6
# No systemd, stemming (libstemmer), Qt, GObject introspection/Vala, SVG
# icon rendering (the compose module) or documentation.
MESON_ARGS_EXTRA=( -Dstemming=false -Dsystemd=false -Dvapi=false -Dqt=false -Dcompose=false
                   -Dapt-support=false -Dgir=false -Dsvg-support=false -Dzstd-support=true
                   -Ddocs=false -Dapidocs=false -Dinstall-docs=false )
# meson looks libyaml up as pkg-config's yaml-0.1, and the libyaml component
# (built with CMake) installs no yaml-0.1.pc. A minimal one, private to this
# build, names the staged library; appstream.pc itself does not require it
# (AppStream strips its Requires.private).
do_configure() {
  mkdir -p "$B/qnx-pc" && cat > "$B/qnx-pc/yaml-0.1.pc" <<PC || return 1
prefix=$PREFIX
libdir=\${prefix}/lib
includedir=\${prefix}/include
Name: LibYAML
Description: Library to parse and emit YAML
Version: 0.2.5
Cflags: -I\${includedir}
Libs: -L\${libdir} -lyaml
PC
  PKG_CONFIG_PATH="$B/qnx-pc${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}" \
  meson setup "$B" "$S" --cross-file "$QNX_MESON_DIR/meson-$arch.ini" \
    --native-file "$QNX_MESON_DIR/meson-native.ini" \
    --prefix="$PREFIX" --buildtype=release --default-library=both "${MESON_ARGS_EXTRA[@]}"
}
