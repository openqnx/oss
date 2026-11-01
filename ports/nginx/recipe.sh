# nginx: HTTP and reverse proxy server, mail and TCP/UDP proxy, stable
# release. Built with its own configure (not autoconf), which does not
# cross-compile on its own: patches/0001 lets --crossbuild=QNX:8.0.0:<cpu>
# answer the probes that would run a test program (type sizes and byte
# order at compile time, run-tests taken from compile-and-link); 0002 is a
# signedness fix for QNX's unsigned blkcnt_t.
VERSION=1.30.6
TARBALL=https://nginx.org/download/nginx-1.30.6.tar.gz
SHA256=3cf46f5ed7504d460125866b7bea99fb357f31d887e2e63b35503310e9b8b6c8
BUILDSYS=custom
# libxcrypt: crypt() for auth_basic password files; QNX's libc has none.
DEPS="openssl pcre2 zlib libxcrypt"
# The cross-GCC driver, not qcc: configure identifies the compiler from
# `$CC -v` (GCC flags, builtin atomics) and qcc reports itself differently
# (docs/porting-hazards.md 21). It compiles for the QNX target either way.
NGX_CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc"
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" || return 1
  # Event method: poll. QNX's epoll emulation has no EPOLLET/EPOLLRDHUP,
  # which nginx's epoll module needs, and there is no kqueue; QNX is an OS
  # nginx does not know, so it gets the generic Unix sources (no sendfile).
  # Paths: FHS-style under /usr/local, logs and temporary files in /var
  # (the package carries /var/log/nginx and /var/tmp/nginx).
  # Workers run as user nobody, group nobody (configure would otherwise pick
  # the group from the build machine's /etc/group). A QNX image made by
  # mkqnximage has the group but no user "nobody": add one, or name a user
  # with the "user" directive, before starting nginx as root.
  ./configure --crossbuild="QNX:8.0.0:$QNX_CPU" --with-cc="$NGX_CC" \
      --prefix="$PREFIX/share/nginx" --sbin-path="$PREFIX/sbin/nginx" \
      --modules-path="$PREFIX/lib/nginx/modules" \
      --conf-path="$PREFIX/etc/nginx/nginx.conf" \
      --error-log-path=/var/log/nginx/error.log --http-log-path=/var/log/nginx/access.log \
      --pid-path=/var/run/nginx.pid --lock-path=/var/run/nginx.lock \
      --http-client-body-temp-path=/var/tmp/nginx/client_body \
      --http-proxy-temp-path=/var/tmp/nginx/proxy --http-fastcgi-temp-path=/var/tmp/nginx/fastcgi \
      --http-uwsgi-temp-path=/var/tmp/nginx/uwsgi --http-scgi-temp-path=/var/tmp/nginx/scgi \
      --user=nobody --group=nobody --with-poll_module --without-select_module \
      --with-threads --with-pcre --with-http_ssl_module --with-http_v2_module \
      --with-http_realip_module --with-http_sub_module --with-http_dav_module \
      --with-http_gunzip_module --with-http_gzip_static_module --with-http_auth_request_module \
      --with-http_secure_link_module --with-http_slice_module --with-http_stub_status_module \
      --with-stream --with-stream_ssl_module --with-stream_realip_module --with-stream_ssl_preread_module \
      --with-mail --with-mail_ssl_module \
      --with-cc-opt="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
      --with-ld-opt="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib -lsocket" || return 1
  # nginx -V prints the configure arguments, compiled in from
  # NGX_CONFIGURE: without the build machine's paths (staging tree, SDP).
  sed -i -e "/NGX_CONFIGURE/s#$STAGE##g" -e "/NGX_CONFIGURE/s#$QNX_HOST/usr/bin/##g" objs/ngx_auto_config.h
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  mkdir -p "$STAGE/var/log/nginx" "$STAGE/var/tmp/nginx"
  install -Dm644 "$B/man/nginx.8" "$STAGE$PREFIX/share/man/man8/nginx.8"
}
