# abiword: the AbiWord word processor (GTK 3) -- documents in its own .abw,
# OpenDocument (.odt), Office Open XML (.docx), Word 97-2003 (.doc, through
# wv), RTF, HTML and text; spelling with enchant. The word processor of the
# desktop (Raspberry Pi OS has LibreOffice Writer in this place; AbiWord is
# the small one).

VERSION=3.0.8
# GNOME GitLab is where AbiWord is developed now (abisource.com is gone); the
# release tag's archive has no generated configure: do_patch makes it (below).
TARBALL=https://gitlab.gnome.org/World/AbiWord/-/archive/release-3.0.8/AbiWord-release-3.0.8.tar.gz
SHA256=7c490e46f0a6143f84d5c7e3fed246f34486ad70cd55f162c792827607cd7e9a
DEPS="glib gtk3 librsvg libgsf wv libxslt libxml2 fribidi boost libpng libjpeg-turbo cairo pango enchant libgcrypt libgpg-error"
RUNTIME_DEPS="librsvg"
BUILDSYS=autotools
# - the default plugins (OpenDocument, Office Open XML) built in the program;
# - no GNOME VFS, Redland (RDF), Evolution Data Server, libical, Champlain;
# - no introspection (gobject-introspection cannot run QNX programs here).
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-gtk2=no --without-gnomevfs
                  --without-redland --without-evolution-data-server --without-libical
                  --without-champlain --disable-introspection --enable-plugins="opendocument openxml" )
# The OpenDocument plugin reads encrypted documents with libgcrypt: configure
# takes libgcrypt-config from PATH, the build machine's ("-L/usr/lib/<host
# multiarch>" in LIBGCRYPT_LIBS, and the plugin linked the host's libc.so.6).
# gpgrt-config of the staged libgpg-error reads the staged libgcrypt.pc
# (as ports/gpgme).
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
# The release tag has no configure: autogen.sh's two steps, run here one by
# one so that a failure stops the build (autogen.sh goes on and returns 0,
# and a failed autoreconf then showed up later as "cannot find required
# auxiliary files: compile missing install-sh config.guess config.sub").
# AM_PATH_LIBGCRYPT comes from the staged libgcrypt's libgcrypt.m4
# (ACLOCAL_PATH): patches/0001 deletes AbiWord's outdated copy, as Debian
# does, and a clean build machine has no system one (libgcrypt20-dev).
do_patch() {
  cd "$SRCROOT" && ./autogen-common.sh && \
  ACLOCAL_PATH="$STAGE$PREFIX/share/aclocal${ACLOCAL_PATH:+:$ACLOCAL_PATH}" autoreconf --force --install || return 1
  for f in compile missing install-sh config.guess config.sub configure; do
    [ -f "$f" ] || { echo "autoreconf left no $f"; return 1; }
  done
}
