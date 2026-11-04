# Tcl: the Tool Command Language -- tclsh8.6 (and tclsh), libtcl8.6.so for
# programs that embed it, libtclstub8.6.a for extensions, the script library
# (lib/tcl8.6) and the bundled packages built with it (Thread, TDBC, itcl,
# SQLite's Tcl binding). Version as in QNX's own package list.
VERSION=8.6.17
TARBALL=https://downloads.sourceforge.net/sourceforge/tcl/tcl8.6.17-src.tar.gz
SHA256=a3903371efcce8a405c5c245d029e9f6850258a60fa3761c4d58995610949b31
BUILDSYS=autotools
SUBDIR=unix
DEPS="zlib"
CONF_ARGS_EXTRA=( --enable-threads --enable-64bit --enable-shared )
# tclConfig.sh, itclConfig.sh and tdbcConfig.sh also describe the build
# tree (*_BUILD_*, *_SRC_DIR: for building extensions against an
# uninstalled Tcl). In the package those name this machine's directories:
# each build-tree value becomes its installed counterpart (TCL_BUILD_LIB_SPEC
# -> TCL_LIB_SPEC's value), the include spec the installed headers, and the
# source and library directories empty.
tcl_fix_config() {
  awk -v ws="$WS" -v inc="-I$PREFIX/include" '
    { line[NR] = $0; if (match($0, /^[A-Za-z_]+=/)) { n = substr($0, 1, RLENGTH - 1); val[n] = substr($0, RLENGTH + 1) } }
    END {
      for (i = 1; i <= NR; i++) {
        l = line[i]
        if (index(l, ws) && match(l, /^[A-Za-z_]+=/)) {
          n = substr(l, 1, RLENGTH - 1); q = substr(l, RLENGTH + 1, 1); if (q != "\"" && q != "\047") q = ""
          c = n; sub(/_BUILD_/, "_", c)
          if (c != n && (c in val) && !index(val[c], ws)) l = n "=" val[c]
          else if (n ~ /_SRC_DIR$|_BUILD_LIBRARY_PATH$/) l = n "=" q q
          else if (n ~ /INCLUDE_SPEC$/) l = n "=" q inc q
        }
        print l
      }
    }' "$1" > "$1.tmp" && mv "$1.tmp" "$1"
}
# tclsh: the unversioned name scripts' #! lines use (as Alpine and QNX ship it).
do_install() {
  make -C "$B" INSTALL_ROOT="$STAGE" install install-private-headers || return 1
  ln -sf tclsh8.6 "$STAGE$PREFIX/bin/tclsh"
  local f
  for f in $(find "$STAGE$PREFIX/lib" -name '*Config.sh'); do tcl_fix_config "$f" || return 1; done
}
