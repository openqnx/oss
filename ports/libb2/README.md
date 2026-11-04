# libb2 0.98.1

> BLAKE2 cryptographic hash library (BLAKE2b, BLAKE2s, BLAKE2bp, BLAKE2sp)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/BLAKE2/libb2/releases/download/v0.98.1/libb2-0.98.1.tar.gz |
| SHA-256 | `53626fddce753c454a3fea581cbbc7fe9bbcf0bc70416d48fdbbf5d87ef6c72e` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libb2`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libb2` (programs, shared libraries, run-time data) and `libb2-dev` (headers, pkg-config/CMake files, static libraries; needs `libb2`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libb2`](https://openqnx.github.io/oss/package/x86_64/libb2.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libb2.html)) · [`libb2-dev`](https://openqnx.github.io/oss/package/x86_64/libb2-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libb2-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libb2` (dependencies come along); one version: `apk add libb2=<version>` | `apk del libb2` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libb2` (dependencies first) | `oss-get remove libb2` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libb2-dev` the same way, by its own name (`apk add libb2-dev`, `oss-get install libb2-dev`, by hand with its own file and list `libb2-dev.list`); remove it before `libb2`, which it needs.

```sh
# on the QNX target, as root
apk add libb2 libb2-dev
apk add qnx-test-libb2 && /usr/local/bin/qnx-port-tests/test_libb2    # its on-target test, from tests-8.0-<arch>
apk del libb2 libb2-dev qnx-test-libb2
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libb2.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libb2.list
rm var/lib/manual/libb2.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libb2` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://github.com/BLAKE2/libb2/releases/download/v0.98.1/libb2-0.98.1.tar.gz
mkdir -p dist src/libb2
curl -fL -o dist/$(basename $URL) $URL
echo "53626fddce753c454a3fea581cbbc7fe9bbcf0bc70416d48fdbbf5d87ef6c72e  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libb2 --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libb2 < ports/libb2/patches/0001-include-path-flag.patch
```

**[`0001-include-path-flag.patch`](patches/0001-include-path-flag.patch)** — Pass the build's src/ directory with -I, not as a bare operand.

> libb2_la_CPPFLAGS lists the build's src/ directory without -I, so every
> compile line gets a stray "../src/" operand. GCC's driver ignores a
> directory operand when compiling; QNX's qcc driver counts it as a second
> input and refuses ("Can't specify -c ... with -o and have multiple files").
> Make it the include path it was meant to be (Makefile.am and the shipped
> Makefile.in alike).

Touches: `src/Makefile.am`, `src/Makefile.in` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libb2 && cd build/$QNX_ARCH/libb2
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libb2/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-static \
    --disable-native \
    --disable-openmp \
    $([ \
    $QNX_ARCH \
    = \
    x86_64 \
    ] \
    && \
    echo \
    --enable-fat \
    || \
    echo \
    --disable-fat)
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libb2: the BLAKE2 reference library -- the BLAKE2b, BLAKE2s, BLAKE2bp and BLAKE2sp hash functions (keyed and unkeyed), from the BLAKE2 authors.
- Never -march=native (that would be the build machine's CPU). On x86_64 a "fat" library: the SSE2/SSSE3/SSE4.1/AVX/XOP variants are all built and the fastest one the CPU supports is picked at run time (cpuid). aarch64 has the portable C code only. OpenMP (for the parallel bp/sp variants) is not in the SDP; they run sequentially.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libb2`.

### 5. What it installs

4 files under `/usr/local`: 2 shared-library entries, 1 headers.

```
/usr/local/lib/libb2.so
/usr/local/lib/libb2.so.1
```

The complete list is `stage/<arch>/.manifest/libb2` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libb2
ls dists/8.0/*/ | grep '^libb2-'
```

This writes `libb2-<ver>-r0.tar.gz` (run time) and `libb2-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libb2
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libb2-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libb2-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libb2`](../../tests/src/) links libb2 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libb2`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libb2
/usr/local/bin/qnx-port-tests/test_libb2
```

```
== libb2 ==
  ok   ...
PASS libb2
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libb2` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libb2`](../../tests/src/) -- a program linked against libb2 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - BLAKE2b-512(\"abc\") = RFC 7693 appendix A
   - BLAKE2s-256(\"abc\") = RFC 7693 appendix B
   - BLAKE2b(\"\")
   - BLAKE2s(00..fe)
   - keyed BLAKE2b(00 01 02)
   - keyed BLAKE2b(00..fe)
   - keyed BLAKE2s(\"\")
   - keyed BLAKE2s(00..fe)
   - keyed BLAKE2bp(00..fe)
   - keyed BLAKE2bp(00 01 02)
   - keyed BLAKE2sp(00..fe)
   - keyed BLAKE2sp(\"\")
   - BLAKE2b in 1+127+127 byte pieces = one-shot
   - BLAKE2sp in 7-byte pieces = one-shot
   - BLAKE2b-256 is not a prefix of BLAKE2b-512
   - BLAKE2b-256(\"abc\")
   - BLAKE2s rejects a 33-byte key

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> libb2`.

## Versions

This recipe builds **0.98.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libb2=<version>`), sources in `ported/libb2/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libb2`; port one with `scripts/port-update libb2`.

<details><summary>recipe.sh</summary>

```bash
# libb2: the BLAKE2 reference library -- the BLAKE2b, BLAKE2s, BLAKE2bp and
# BLAKE2sp hash functions (keyed and unkeyed), from the BLAKE2 authors.
VERSION=0.98.1
TARBALL=https://github.com/BLAKE2/libb2/releases/download/v0.98.1/libb2-0.98.1.tar.gz
SHA256=53626fddce753c454a3fea581cbbc7fe9bbcf0bc70416d48fdbbf5d87ef6c72e
BUILDSYS=autotools
# Never -march=native (that would be the build machine's CPU). On x86_64 a
# "fat" library: the SSE2/SSSE3/SSE4.1/AVX/XOP variants are all built and
# the fastest one the CPU supports is picked at run time (cpuid). aarch64
# has the portable C code only. OpenMP (for the parallel bp/sp variants) is
# not in the SDP; they run sequentially.
CONF_ARGS_EXTRA=( --disable-static --disable-native --disable-openmp
                  $([ "$QNX_ARCH" = x86_64 ] && echo --enable-fat || echo --disable-fat) )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
