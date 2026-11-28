# Xarchiver: a GTK 3 archive manager -- opens, creates, extracts and tests
# archives by running the command-line archivers it finds in PATH (tar, 7zz,
# zip, xz, bzip2, zstd, lz4, lzip, gzip, cpio, ar, ...), with a file list,
# drag and drop, and non-interactive modes the file manager uses
# ("xarchiver --extract-to=DIR ARCHIVE", --ensure-directory, --compress).
# "xarchiver --info" lists the archive types the installed programs give.
VERSION=0.5.4.27; GIT=https://github.com/ib/xarchiver.git; REF=0.5.4.27
# intltool: a build tool here (the translated .desktop file and the message
# catalogues are made with intltool-merge/-update).
DEPS="glib gtk3 intltool gettext"
# The archivers it drives: what is installed decides the formats offered.
# GNU tar for tar and every tar.* (its options are GNU tar's), 7zip for 7z, zip
# (reading), rar (reading), iso and more, zip for writing zip archives.
RUNTIME_DEPS="tar 7zip zip xz bzip2 zstd lz4 lzip hicolor-icon-theme adwaita-icon-theme"
BUILDSYS=autotools
# The repository ships its configure script. intltool's programs are Perl
# scripts, and the staged ones are the target's (they start with
# #!/usr/local/bin/perl, which the build machine does not have). configure
# calls "intltool-update --version" by name before it looks for the programs,
# so setting INTLTOOL_UPDATE is not enough: three wrappers in the build
# directory run the staged scripts with the build machine's perl, and that
# directory is first on PATH for configure, make and the install.
_xa_hostbin() {
  local t
  mkdir -p "$B/hostbin" || return 1
  for t in intltool-update intltool-merge intltool-extract; do
    printf '#!/bin/sh\nexec /usr/bin/perl "%s" "$@"\n' "$STAGE$PREFIX/bin/$t" > "$B/hostbin/$t" && chmod +x "$B/hostbin/$t" || return 1
  done
}
# The cross environment is the one scripts/qnxbuild gives every Autotools
# component (compiler, staging before the SDP, the libraries QNX splits out of
# libc); a hook replaces the whole step, so it is spelled out here.
do_configure() {
  _xa_hostbin || return 1
  # The pre-built HTML help is in the source tree; a git checkout gives it the
  # same time as its DocBook source, and make then rebuilds it with rules that
  # only work inside the source directory ("cp: cannot stat 'xarchiver.css'").
  # Mark the shipped pages as up to date so they are installed as they are.
  touch "$S"/doc/html/* 2>/dev/null
  ( cd "$B" && PATH="$B/hostbin:$PATH" INTLTOOL_PERL=/usr/bin/perl \
    CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    CXXFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include $CFLAGS_APPEND" \
    LDFLAGS="-Wl,--push-state,--no-as-needed,-lc,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify $LDFLAGS_APPEND" \
    "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}" )
}
do_build() { PATH="$B/hostbin:$PATH" make -C "$B" -j"$(nproc)"; }
# The wrappers were written at configure time, with the staging tree's path:
# during the install $STAGE names this component's package root.
do_install() { PATH="$B/hostbin:$PATH" make -C "$B" DESTDIR="$STAGE" install; }
# GTK 3 (not --enable-gtk2). The HTML manual (Help > Contents) is the pre-built
# one from the repository; the icons and the Thunar archive plugin script
# (libexec/thunar-archive-plugin/xarchiver.tap) are installed.
CONF_ARGS_EXTRA=( --disable-gtk2 --enable-doc --enable-icons --enable-plugin )
