VERSION=0.2.13; GIT=https://github.com/tlwg/libdatrie.git; REF=v0.2.13
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doxygen-doc )
# Two problems, both from autoconf-archive not being installed here:
# build-aux is kept out of git, and configure.ac calls AX_COMPARE_VERSION --
# which the shipped ./configure contains *unexpanded*, so the shell cannot
# even parse it, disabled doxygen or not. Patch 0001 replaces that call;
# here we drop in automake's own auxiliary scripts and regenerate.
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
