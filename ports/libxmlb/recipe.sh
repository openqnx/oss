# libxmlb: library to help create and query binary XML blobs (mmap-able
# compiled XML with XPath-like queries). Dependency of AppStream, which
# libadwaita needs for its "about" dialog from AppStream metadata.
VERSION=0.3.24; GIT=https://github.com/hughsie/libxmlb.git; REF=0.3.24
DEPS="glib xz zstd"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgtkdoc=false -Dintrospection=false -Dtests=false -Dstemmer=false
                   -Dcli=true -Dlzma=enabled -Dzstd=enabled )
