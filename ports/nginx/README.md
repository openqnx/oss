# nginx 1.30.6

> HTTP web server, reverse proxy, load balancer and TCP/UDP/mail proxy

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://nginx.org/download/nginx-1.30.6.tar.gz |
| SHA-256 | `3cf46f5ed7504d460125866b7bea99fb357f31d887e2e63b35503310e9b8b6c8` |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_nginx`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `nginx` (programs, shared libraries, run-time data) and `nginx-dev` (headers, pkg-config/CMake files, static libraries; needs `nginx`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`nginx`](https://openqnx.github.io/oss/package/x86_64/nginx.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nginx.html)) · [`nginx-dev`](https://openqnx.github.io/oss/package/x86_64/nginx-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nginx-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add nginx` (dependencies come along); one version: `apk add nginx=<version>` | `apk del nginx` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install nginx` (dependencies first) | `oss-get remove nginx` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`nginx-dev` the same way, by its own name (`apk add nginx-dev`, `oss-get install nginx-dev`, by hand with its own file and list `nginx-dev.list`); remove it before `nginx`, which it needs.

```sh
# on the QNX target, as root
apk add nginx nginx-dev
apk add qnx-test-nginx && /usr/local/bin/qnx-port-tests/test_nginx    # its on-target test, from tests-8.0-<arch>-m-z
apk del nginx nginx-dev qnx-test-nginx
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/nginx.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/nginx.list
rm var/lib/manual/nginx.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nginx` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl pcre2 zlib libxcrypt
```

<details><summary>Full build chain (5)</summary>

[`bzip2`](../bzip2/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://nginx.org/download/nginx-1.30.6.tar.gz
mkdir -p dist src/nginx
curl -fL -o dist/$(basename $URL) $URL
echo "3cf46f5ed7504d460125866b7bea99fb357f31d887e2e63b35503310e9b8b6c8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/nginx --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/nginx < ports/nginx/patches/0001-crossbuild-without-running-tests.patch
patch -p1 -d src/nginx < ports/nginx/patches/0002-st-blocks-unsigned.patch
```

**[`0001-crossbuild-without-running-tests.patch`](patches/0001-crossbuild-without-running-tests.patch)** — configure: support --crossbuild without running test programs

> nginx's configure compiles small test programs and runs them: for type
> sizes (auto/types/sizeof), byte order (auto/endianness) and the
> run-tests in auto/feature (ngx_feature_run=yes/bug/value). In a cross
> build none of them can run, so configure stops with "can not detect int
> size". --crossbuild=<system>:<release>:<machine> also set the machine to
> i386 regardless of the target.
>
> With --crossbuild=SYS:REL:MACHINE (three fields) configure now takes the
> machine from the third field and sets NGX_CROSSBUILD=YES, in which case:
> sizes are found at compile time (a negative array size does not compile),
> byte order from __BYTE_ORDER__, a run-test that compiles and links counts
> as working unless its name is in NGX_CROSS_NOT_WORKING, a "bug" probe as
> not affected, and a value probe takes NGX_CROSS_VALUE_<name> or counts as
> absent. Native builds are unchanged.

Touches: `auto/endianness`, `auto/feature`, `auto/types/sizeof`, `configure` · Upstream status: QNX-specific

**[`0002-st-blocks-unsigned.patch`](patches/0002-st-blocks-unsigned.patch)** — Compute file sizes from st_blocks as off_t (unsigned blkcnt_t on QNX)

> QNX's blkcnt_t (struct stat st_blocks) is unsigned (FreeBSD's is
> signed, glibc's blkcnt_t is signed too), so st_blocks * 512 compared with
> or chosen against the signed off_t st_size is a signed/unsigned
> comparison, which nginx's -Werror build refuses (ngx_file.c). The block
> count of a file is far below off_t's range: compute it as off_t.

Touches: `src/os/unix/ngx_files.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/nginx; B=build/$QNX_ARCH/nginx; mkdir -p $B
```

```bash
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
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install || return 1
  mkdir -p "$STAGE/var/log/nginx" "$STAGE/var/tmp/nginx"
  install -Dm644 "$B/man/nginx.8" "$STAGE$PREFIX/share/man/man8/nginx.8"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- nginx: HTTP and reverse proxy server, mail and TCP/UDP proxy, stable release. Built with its own configure (not autoconf), which does not cross-compile on its own: patches/0001 lets --crossbuild=QNX:8.0.0:<cpu> answer the probes that would run a test program (type sizes and byte order at compile time, run-tests taken from compile-and-link); 0002 is a signedness fix for QNX's unsigned blkcnt_t.
- libxcrypt: crypt() for auth_basic password files; QNX's libc has none.
- The cross-GCC driver, not qcc: configure identifies the compiler from `$CC -v` (GCC flags, builtin atomics) and qcc reports itself differently (docs/porting-hazards.md 21). It compiles for the QNX target either way.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nginx`.

### 5. What it installs

19 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/nginx
```

The complete list is `stage/<arch>/.manifest/nginx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nginx
ls dists/8.0/*/ | grep '^nginx-'
```

This writes `nginx-<ver>-r0.tar.gz` (run time) and `nginx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nginx
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nginx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nginx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_nginx`](../../tests/src/) links nginx the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-nginx`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-nginx
/usr/local/bin/qnx-port-tests/test_nginx
```

```
== nginx ==
  ok   ...
PASS nginx
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_nginx` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_nginx`](../../tests/src/) -- a program linked against nginx the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - nginx -V: version 1.30.6
   - nginx -V: built with OpenSSL and the SSL module
   - self-signed certificate written (OpenSSL)
   - nginx -t accepts the configuration
   - nginx -t rejects an unknown directive
   - nginx is up and answering on 127.0.0.1
   - GET /hello.txt: 200 and the file byte for byte
   - Content-Type: text/plain
   - Content-Length: 6000
   - Server: nginx/1.30.6
   - GET /nope.txt: 404
   - return 200 with a body
   - return 301: Location ends in /hello.txt
   - PCRE rewrite /r/hello.text -> /hello.txt
   - gzip on request: Content-Encoding gzip, gzip stream
   - gzip: compressed response is smaller than the file
   - HTTPS GET /hello.txt: 200 and the file byte for byte
   - HTTPS: TLS 1.3 negotiated
   - graceful stop (SIGQUIT): exit 0

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | nginx 1.30.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/nginx-1.30.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | nginx 1.30.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/nginx-1.30.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.30.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nginx=<version>`), sources in `ported/nginx/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nginx`; port one with `scripts/port-update nginx`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
