# QDirStat: Qt 6 graphical disk usage statistics (directory tree with sizes,
# treemap, file-type and file-age statistics, cleanup actions).
# qmake project, built with the host Qt's qmake (hosttools/qt6-<version>)
# against the ported Qt 6 for the target, mkspec qnx-<arch>-qcc. The staged
# bin/target_qt.conf names the host Qt by a path relative to where the
# workspace once was, so the build writes a qt.conf of its own with
# absolute paths. no_qt_rpath: qmake would give the program a run path to
# Qt's library directory, which this qt.conf names in the staging tree.
# -lintl: the translations go through gettext, whose staged <libintl.h>
# maps dgettext() to libintl's libintl_dgettext() (as qnx-ports links it).
# Built in a copy of the source ($B/tree): the .pro files install the
# program and the man pages from the source directory (an in-source build).
# QNX changes from qnx-ports (patches/0001).
VERSION=2.0; GIT=https://github.com/shundhammer/qdirstat.git; REF=2.0
DEPS="qt6 zlib gettext"
BUILDSYS=custom
# qdirstat-cache-writer (scripts/, Perl) is installed too, with the target's
# perl in its #! line (upstream: /usr/bin/perl, which QNX does not have).
RUNTIME_DEPS="perl"
QT_VER=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/qt6/recipe.sh")
do_configure() {
  local spec; case "$QNX_ARCH" in aarch64le) spec=qnx-aarch64le-qcc ;; x86_64) spec=qnx-x86-64-qcc ;; esac
  cat > "$B/qt.conf" <<Q
[Paths]
Prefix=$STAGE$PREFIX
HostPrefix=$WS/hosttools/qt6-$QT_VER
HostBinaries=bin
HostLibraries=lib
HostLibraryExecutables=libexec
HostData=$STAGE$PREFIX
TargetSpec=$spec
HostSpec=linux-g++
Q
  rm -rf "$B/tree" && cp -a "$S" "$B/tree" && rm -rf "$B/tree/.git" || return 1
  cd "$B/tree" && "$WS/hosttools/qt6-$QT_VER/bin/qmake6" -qtconf "$B/qt.conf" qdirstat.pro \
    INSTALL_PREFIX="$PREFIX" "QMAKE_LFLAGS+=-Wl,-rpath-link,$STAGE$PREFIX/lib" \
    "CONFIG+=no_qt_rpath" "LIBS+=-lintl"
}
do_build() { make -C "$B/tree" -j"$(nproc)"; }
do_install() {
  make -C "$B/tree" install INSTALL_ROOT="$STAGE" && \
  sed -i '1s|^#!/usr/bin/perl|#!/usr/local/bin/perl|' "$STAGE$PREFIX/bin/qdirstat-cache-writer"
}
