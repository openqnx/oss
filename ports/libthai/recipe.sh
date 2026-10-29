VERSION=0.1.29; GIT=https://github.com/tlwg/libthai.git; REF=v0.1.29
DEPS="libdatrie"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doxygen-doc )
# Same two problems as libdatrie: build-aux is kept out of git, and the shipped
# ./configure contains an unexpanded AX_COMPARE_VERSION that the shell cannot
# parse. Patch 0001 replaces that call; here we drop in automake's own
# auxiliary scripts and regenerate.
# (autogen.sh is no good either -- it runs ./configure for the build host,
# and the cross configure then refuses with "already configured".)
do_patch() {
  local aux; aux=$(ls -d /usr/share/automake-*/ 2>/dev/null | head -1)
  [ -n "$aux" ] || { echo "no automake data directory found"; return 1; }
  mkdir -p "$SRCROOT/build-aux"
  local f
  for f in config.guess config.sub compile missing install-sh depcomp ar-lib; do
    [ -e "$aux/$f" ] && cp -f "$aux/$f" "$SRCROOT/build-aux/$f"
  done
  chmod +x "$SRCROOT/build-aux/"*
  ( cd "$SRCROOT" && autoreconf -fi )
}
