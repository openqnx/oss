# Ruby: the interpreter (ruby, and the irb/gem/bundle/erb/rake/rdoc/ri
# front ends), libruby.so.3.4, the standard library with its C extensions
# (json, zlib, openssl, psych over libyaml, fiddle over libffi, readline via
# reline, io/console, socket, ...) and the default and bundled gems.
# QNX's own package list has 3.4.7; this is the latest 3.4 release.
VERSION=3.4.12
TARBALL=https://cache.ruby-lang.org/pub/ruby/3.4/ruby-3.4.12.tar.gz
SHA256=5e5d85ad93b0adcccbad47733680f4892bd0078e3342437f7babce01c1872479
BUILDSYS=autotools
# libxcrypt: crypt() (String#crypt), which QNX's libc lacks.
DEPS="zlib openssl libyaml libffi gmp libxcrypt"
# Cache answers for what configure cannot run in a cross build, as QNX's
# own (native) build sets them: isnan/isinf are macros; no openat (it would
# use O_DIRECTORY paths QNX does not have); no thread-local storage
# specifier (QNX's TLS model breaks the VM's).
CONF_ENV=( ac_cv_func_isnan=yes ac_cv_func_isinf=yes ac_cv_func_malloc_trim=no
           ac_cv_func_openat=no rb_cv_tls_specifier=none )
# A cross build runs a Ruby of the same version on the build machine
# (BASERUBY: it generates sources and runs mkmf for the extensions with the
# target's rbconfig). It is built first, from the same source, once, with
# only what that needs: no openssl extension (the build machine's OpenSSL
# headers did not match what its configure found), no documentation.
HOSTRUBY_DIR="$WS/build/host/ruby-$VERSION"
ruby_host_build() {
  [ -x "$HOSTRUBY_DIR/inst/bin/ruby" ] && return 0
  rm -rf "$HOSTRUBY_DIR"; mkdir -p "$HOSTRUBY_DIR/b"
  ( cd "$HOSTRUBY_DIR/b" && env -u MAKEFLAGS -u CC -u CXX -u CFLAGS -u CPPFLAGS -u LDFLAGS -u PKG_CONFIG_PATH -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
      "$S/configure" --prefix="$HOSTRUBY_DIR/inst" --disable-install-doc --disable-shared --with-out-ext=openssl \
    && make -j"$(nproc)" && make install && [ -x "$HOSTRUBY_DIR/inst/bin/ruby" ] ) || { rm -rf "$HOSTRUBY_DIR"; return 1; }
}
do_configure() {
  ruby_host_build || return 1
  local cor; case "$QNX_CPU" in x86_64) cor=amd64 ;; aarch64*) cor=arm64 ;; esac
  # the driver's autotools configure (scripts/qnxbuild), with the arguments
  # Ruby needs: shared libruby, no RPATH, the coroutine implementation for
  # the CPU (configure cannot tell for nto), the build machine's ruby.
  # The socket extension needs to know whether recvmsg(MSG_PEEK) of a
  # passed descriptor allocates one (it cannot run its test program in a
  # cross build, and stops): ext/socket/extconf.rb's test program, run on
  # QNX 8, says it does, as on Linux.
  # --with-setup=Setup: configure picks the first ext/Setup.<os> whose name
  # starts $target_os, and "nto-qnx..." starts with "nt": it took Windows'
  # Setup.nt, which links a list of extensions statically into the ruby
  # program, and the link failed on their symbols.
  ( cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
    CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    CXXFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include -I$QNX_TARGET/usr/include" \
    LDFLAGS="-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state -L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
    env "${CONF_ENV[@]}" "$S/configure" --host="$QNX_TRIPLE" --prefix="$PREFIX" \
      --enable-shared --disable-rpath --disable-install-doc --enable-pthread \
      --with-coroutine=$cor --with-baseruby="$HOSTRUBY_DIR/inst/bin/ruby" \
      --with-opt-dir="$STAGE$PREFIX" --enable-close-fds-by-recvmsg-with-peek \
      --with-setup=Setup )
}
# The SDP's environment sets MAKEFLAGS=-I$QNX_TARGET/usr/include. Ruby's
# installer (tool/rbinstall.rb) reads the single-letter make flags out of it
# and finds the "n" of ".../qnx805/..." -- make -n: it ran as a dry run,
# printed every "installing ..." line, wrote nothing, and said "Installed
# under" an empty directory. Install without it.
# A cross build runs the build machine's ruby where a native one runs
# miniruby, so the target's miniruby is made explicitly.
do_build() {
  make -C "$B" -j"$(nproc)" && make -C "$B" miniruby
}
# miniruby -- the interpreter without extensions or rubygems that the build
# itself uses -- is installed too: a small Ruby for scripts that need only
# the core classes (and a way to tell a core problem from an extension one).
do_install() {
  env -u MAKEFLAGS make -C "$B" DESTDIR="$STAGE" install
  install -m755 "$B/miniruby" "$STAGE$PREFIX/bin/miniruby"
  # rbconfig.rb records the configure arguments; the build machine's ruby
  # (--with-baseruby) means nothing on the target.
  sed -i "s| '--with-baseruby=[^']*'||" "$STAGE$PREFIX"/lib/ruby/3.4.0/*-nto-qnx/rbconfig.rb
}
