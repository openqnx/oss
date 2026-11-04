# ruby 3.4.12

> Ruby programming language: interpreter, standard library, extensions and gems

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://cache.ruby-lang.org/pub/ruby/3.4/ruby-3.4.12.tar.gz |
| SHA-256 | `5e5d85ad93b0adcccbad47733680f4892bd0078e3342437f7babce01c1872479` |
| Build system | GNU Autotools |
| QNX patches | 5 |
| Build dependencies | [`zlib`](../zlib/) [`openssl`](../openssl/) [`libyaml`](../libyaml/) [`libffi`](../libffi/) [`gmp`](../gmp/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`gmp`](../gmp/) [`libffi`](../libffi/) [`libxcrypt`](../libxcrypt/) [`libyaml`](../libyaml/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ruby`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ruby` (programs, shared libraries, run-time data) and `ruby-dev` (headers, pkg-config/CMake files, static libraries; needs `ruby`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ruby`](https://openqnx.github.io/oss/package/x86_64/ruby.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ruby.html)) · [`ruby-dev`](https://openqnx.github.io/oss/package/x86_64/ruby-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ruby-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ruby` (dependencies come along); one version: `apk add ruby=<version>` | `apk del ruby` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ruby` (dependencies first) | `oss-get remove ruby` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ruby-dev` the same way, by its own name (`apk add ruby-dev`, `oss-get install ruby-dev`, by hand with its own file and list `ruby-dev.list`); remove it before `ruby`, which it needs.

```sh
# on the QNX target, as root
apk add ruby ruby-dev
apk add qnx-test-ruby && /usr/local/bin/qnx-port-tests/test_ruby    # its on-target test, from tests-8.0-<arch>-m-z
apk del ruby ruby-dev qnx-test-ruby
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ruby.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ruby.list
rm var/lib/manual/ruby.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ruby` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib openssl libyaml libffi gmp libxcrypt
```

### 2. Get the upstream source

```bash
URL=https://cache.ruby-lang.org/pub/ruby/3.4/ruby-3.4.12.tar.gz
mkdir -p dist src/ruby
curl -fL -o dist/$(basename $URL) $URL
echo "5e5d85ad93b0adcccbad47733680f4892bd0078e3342437f7babce01c1872479  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/ruby --strip-components=1
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ruby < ports/ruby/patches/0001-qnx-setmode.patch
patch -p1 -d src/ruby < ports/ruby/patches/0002-howmany.patch
patch -p1 -d src/ruby < ports/ruby/patches/0003-socket-bintime.patch
patch -p1 -d src/ruby < ports/ruby/patches/0004-libruby-soname.patch
patch -p1 -d src/ruby < ports/ruby/patches/0005-qnx-thread-stack.patch
```

**[`0001-qnx-setmode.patch`](patches/0001-qnx-setmode.patch)** — QNX 8 has O_BINARY/O_TEXT but no setmode(): make it a no-op.

> QNX's <fcntl.h> defines O_BINARY and O_TEXT (both 0), so io.c compiles
> its DOS text/binary mode switching, which calls setmode(). QNX 8's libc
> has no setmode() (unix.h no longer declares it): libruby failed to link.
> Both modes are the same on QNX, so setmode() is a no-op that reports
> binary mode.

Touches: `io.c` · Upstream status: QNX-specific

**[`0002-howmany.patch`](patches/0002-howmany.patch)** — Define howmany() where <sys/param.h> lacks it (QNX).

> thread.c sizes its fd_set copies with howmany(), the BSD <sys/param.h>
> macro, which QNX's headers do not define: the link failed with undefined
> references to howmany. Define it, rounding up as BSD does, where it is
> missing (QNX's own port defines it as A/B + 1, which over-allocates by
> one word when the division is exact).

Touches: `thread.c` · Upstream status: Pending

**[`0003-socket-bintime.patch`](patches/0003-socket-bintime.patch)** — Declare struct bintime for SCM_BINTIME on QNX.

> QNX 8's <sys/socket.h> (the io-sock network stack, from FreeBSD) defines
> SCM_BINTIME, so ancdata.c compiles its decoder for that timestamp control
> message, but struct bintime is declared only in the stack's own headers
> (<net/bpf.h>, <devs/sys/time.h>), which a socket program does not
> include: the socket extension failed to compile. Declare the struct with
> the layout of those headers. (QNX's own Ruby port carries the same
> definition.)

Touches: `ext/socket/ancdata.c` · Upstream status: QNX-specific

**[`0004-libruby-soname.patch`](patches/0004-libruby-soname.patch)** — Give libruby a soname on QNX, as on Linux.

> On QNX (target_os nto-qnx*) configure gives libruby no soname: libruby.so.3.4.12
> is linked without -soname, so the ruby program records NEEDED libruby.so -- the
> unversioned development link -- and, packaged the usual way (the .so link in
> the -dev package), ruby does not start: "Could not load library libruby.so".
> QNX's linker and loader handle sonames as on Linux: take the Linux branch
> (soname libruby.so.3.4, aliases libruby.so.3.4 and libruby.so).
> The generated configure is changed alongside configure.ac (the release
> archive ships it and the build does not regenerate it).

Touches: `configure`, `configure.ac` · Upstream status: Pending

**[`0005-qnx-thread-stack.patch`](patches/0005-qnx-thread-stack.patch)** — Find a thread's stack on QNX through __tls(), so threads can start.

> Every Thread.new crashed on QNX ([BUG] Segmentation fault): a new native
> thread calls native_thread_init_stack(), which without STACKADDR_AVAILABLE
> raises NotImplementedError ("ruby engine can initialize only in the main
> thread") from a thread that has no Ruby context yet. QNX has none of the
> functions get_stack() knows (pthread_getattr_np, pthread_attr_get_np, ...),
> but every thread's local storage (<sys/storage.h>, __tls()) records its
> stack base and sits at the top of the stack: use that. With it threads
> start, and stack overflow in a thread raises SystemStackError.

Touches: `thread_pthread.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ruby; B=build/$QNX_ARCH/ruby; mkdir -p $B
```

```bash
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
```

```bash
do_build() {
  make -C "$B" -j"$(nproc)" && make -C "$B" miniruby
}
```

```bash
do_install() {
  env -u MAKEFLAGS make -C "$B" DESTDIR="$STAGE" install
  install -m755 "$B/miniruby" "$STAGE$PREFIX/bin/miniruby"
  # rbconfig.rb records the configure arguments; the build machine's ruby
  # (--with-baseruby) means nothing on the target.
  sed -i "s| '--with-baseruby=[^']*'||" "$STAGE$PREFIX"/lib/ruby/3.4.0/*-nto-qnx/rbconfig.rb
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Ruby: the interpreter (ruby, and the irb/gem/bundle/erb/rake/rdoc/ri front ends), libruby.so.3.4, the standard library with its C extensions (json, zlib, openssl, psych over libyaml, fiddle over libffi, readline via reline, io/console, socket, ...) and the default and bundled gems. QNX's own package list has 3.4.7; this is the latest 3.4 release.
- libxcrypt: crypt() (String#crypt), which QNX's libc lacks.
- Cache answers for what configure cannot run in a cross build, as QNX's own (native) build sets them: isnan/isinf are macros; no openat (it would use O_DIRECTORY paths QNX does not have); no thread-local storage specifier (QNX's TLS model breaks the VM's).
- A cross build runs a Ruby of the same version on the build machine (BASERUBY: it generates sources and runs mkmf for the extensions with the target's rbconfig). It is built first, from the same source, once, with only what that needs: no openssl extension (the build machine's OpenSSL headers did not match what its configure found), no documentation.
- The SDP's environment sets MAKEFLAGS=-I$QNX_TARGET/usr/include. Ruby's installer (tool/rbinstall.rb) reads the single-letter make flags out of it and finds the "n" of ".../qnx805/..." -- make -n: it ran as a dry run, printed every "installing ..." line, wrote nothing, and said "Installed under" an empty directory. Install without it. A cross build runs the build machine's ruby where a native one runs miniruby, so the target's miniruby is made explicitly.
- miniruby -- the interpreter without extensions or rubygems that the build itself uses -- is installed too: a small Ruby for scripts that need only the core classes (and a way to tell a core problem from an extension one).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ruby`.

### 5. What it installs

2730 files under `/usr/local`: 3 shared-library entries, 15 programs, 193 headers.

```
/usr/local/lib/libruby.so
/usr/local/lib/libruby.so.3.4
/usr/local/lib/libruby.so.3.4.12
/usr/local/bin/bundle
/usr/local/bin/bundler
/usr/local/bin/erb
/usr/local/bin/gem
/usr/local/bin/irb
/usr/local/bin/miniruby
/usr/local/bin/racc
/usr/local/bin/rake
/usr/local/bin/rbs
/usr/local/bin/rdbg
/usr/local/bin/rdoc
/usr/local/bin/ri
/usr/local/bin/ruby
...
```

The complete list is `stage/<arch>/.manifest/ruby` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ruby
ls dists/8.0/*/ | grep '^ruby-'
```

This writes `ruby-<ver>-r0.tar.gz` (run time) and `ruby-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ruby
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ruby-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ruby-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ruby`](../../tests/src/) links ruby the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ruby`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ruby
/usr/local/bin/qnx-port-tests/test_ruby
```

```
== ruby ==
  ok   ...
PASS ruby
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ruby` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ruby`](../../tests/src/) -- a program linked against ruby the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> ruby`.

## Versions

This recipe builds **3.4.12**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ruby=<version>`), sources in `ported/ruby/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ruby`; port one with `scripts/port-update ruby`.

<details><summary>recipe.sh</summary>

```bash
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
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
