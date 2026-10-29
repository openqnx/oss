# chrony from upstream at the commit qnx-ports' QNX support is based on; the
# patch adds a QNX system driver (sys_qnx.c: clock slewing through
# ClockAdjust()) and a QNX case in configure.
VERSION=4.8-git-20260226; GIT=https://github.com/mlichvar/chrony.git; COMMIT=d622b222a9acbda3a4c842beeb593e7075c32bec
BUILDSYS=custom
# chrony's configure is hand-written (not autoconf) and picks the system
# driver from the OS name it is given.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" \
  CPPFLAGS="-I$STAGE$PREFIX/include" LDFLAGS="-L$STAGE$PREFIX/lib -lsocket" \
  ./configure --host-system=QNX --host-release=8.0.0 --host-machine="$QNX_CPU" --prefix="$PREFIX" --without-nettle --without-gnutls --without-nss --without-tomcrypt \
     --disable-readline --without-editline --without-seccomp --without-libcap --disable-privdrop --disable-pps; }
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
