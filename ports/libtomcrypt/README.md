# libtomcrypt 1.18.2

> Public-domain modular cryptographic toolkit written in C

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libtom/libtomcrypt/releases/download/v1.18.2/crypt-1.18.2.tar.xz |
| SHA-256 | `96ad4c3b8336050993c5bc2cf6c057484f2b0f9f763448151567fbab5e767b84` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`libtommath`](../libtommath/) |
| Run-time dependencies | [`libtommath`](../libtommath/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libtomcrypt`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libtomcrypt` (programs, shared libraries, run-time data) and `libtomcrypt-dev` (headers, pkg-config/CMake files, static libraries; needs `libtomcrypt`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libtomcrypt`](https://openqnx.github.io/oss/package/x86_64/libtomcrypt.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libtomcrypt.html)) · [`libtomcrypt-dev`](https://openqnx.github.io/oss/package/x86_64/libtomcrypt-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libtomcrypt-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libtomcrypt` (dependencies come along); one version: `apk add libtomcrypt=<version>` | `apk del libtomcrypt` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libtomcrypt` (dependencies first) | `oss-get remove libtomcrypt` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libtomcrypt-dev` the same way, by its own name (`apk add libtomcrypt-dev`, `oss-get install libtomcrypt-dev`, by hand with its own file and list `libtomcrypt-dev.list`); remove it before `libtomcrypt`, which it needs.

```sh
# on the QNX target, as root
apk add libtomcrypt libtomcrypt-dev
apk add qnx-test-libtomcrypt && /usr/local/bin/qnx-port-tests/test_libtomcrypt    # its on-target test, from tests-8.0-<arch>
apk del libtomcrypt libtomcrypt-dev qnx-test-libtomcrypt
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libtomcrypt.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libtomcrypt.list
rm var/lib/manual/libtomcrypt.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtomcrypt` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libtommath
```

### 2. Get the upstream source

```bash
URL=https://github.com/libtom/libtomcrypt/releases/download/v1.18.2/crypt-1.18.2.tar.xz
mkdir -p dist src/libtomcrypt
curl -fL -o dist/$(basename $URL) $URL
echo "96ad4c3b8336050993c5bc2cf6c057484f2b0f9f763448151567fbab5e767b84  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libtomcrypt --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libtomcrypt; B=build/$QNX_ARCH/libtomcrypt; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/"; }
```

```bash
do_build() { _ltcmake libtomcrypt.a && cd "$B" && \
  qcc -V$QCC_TARGET -shared -Wl,-soname,libtomcrypt.so.1 -o libtomcrypt.so.1.0.1 \
    -Wl,--whole-archive libtomcrypt.a -Wl,--no-whole-archive -L"$STAGE$PREFIX/lib" -ltommath; }
```

```bash
do_install() { _ltcmake DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/lib/libtomcrypt.a" && \
  sed -i 's,^Libs:.*,&\nLibs.private: -ltommath,; s,^Cflags:.*,Cflags: -I${includedir} -DUSE_LTM -DLTM_DESC,' "$STAGE$PREFIX/lib/pkgconfig/libtomcrypt.pc" && \
  install -m755 "$B/libtomcrypt.so.1.0.1" "$STAGE$PREFIX/lib/" && \
  ln -sf libtomcrypt.so.1.0.1 "$STAGE$PREFIX/lib/libtomcrypt.so.1" && ln -sf libtomcrypt.so.1 "$STAGE$PREFIX/lib/libtomcrypt.so"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- makefile.shared needs a libtool that knows the QNX cross compiler; the plain makefile.unix (static archive only) is portable make. Its objects are compiled position-independent and then linked into the shared library libtool would have made: libtomcrypt.so.1.0.1, soname libtomcrypt.so.1 (-version-info 1:1, as in makefile_include.mk). USE_LTM/LTM_DESC builds the libtommath math descriptor (ltm_desc) for RSA, DSA, DH and ECC; the library then needs libtommath.so.1.
- Shared library only (as libtommath): the static archive is a build step.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtomcrypt`.

### 5. What it installs

17 files under `/usr/local`: 3 shared-library entries, 13 headers.

```
/usr/local/lib/libtomcrypt.so
/usr/local/lib/libtomcrypt.so.1
/usr/local/lib/libtomcrypt.so.1.0.1
```

The complete list is `stage/<arch>/.manifest/libtomcrypt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libtomcrypt
ls dists/8.0/*/ | grep '^libtomcrypt-'
```

This writes `libtomcrypt-<ver>-r0.tar.gz` (run time) and `libtomcrypt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libtomcrypt
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libtomcrypt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libtomcrypt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libtomcrypt`](../../tests/src/) links libtomcrypt the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libtomcrypt`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libtomcrypt
/usr/local/bin/qnx-port-tests/test_libtomcrypt
```

```
== libtomcrypt ==
  ok   ...
PASS libtomcrypt
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libtomcrypt` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libtomcrypt`](../../tests/src/) -- a program linked against libtomcrypt the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - register sha256, aes, sprng
   - SHA-256(\"abc\") (FIPS 180-2)
   - AES-128 FIPS-197 C.1 encrypt
   - AES-128 decrypt round trip
   - HMAC-SHA-256 (RFC 4231 case 2)
   - Base64(\"foobar\") = Zm9vYmFy (RFC 4648)
   - sha256_test() self-test
   - AES self-test
   - GCM self-test
   - ChaCha20-Poly1305 self-test
   - PRNG seeded from the system (/dev/urandom)
   - RSA-1024 key generated (ltm_desc)
   - RSA PKCS#1 v1.5 signature, 128 bytes
   - signature verifies
   - a changed hash does not verify

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libtomcrypt 1.18.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libtomcrypt-1.18.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libtomcrypt 1.18.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libtomcrypt-1.18.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.18.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libtomcrypt=<version>`), sources in `ported/libtomcrypt/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libtomcrypt`; port one with `scripts/port-update libtomcrypt`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.18.2
TARBALL=https://github.com/libtom/libtomcrypt/releases/download/v1.18.2/crypt-1.18.2.tar.xz
SHA256=96ad4c3b8336050993c5bc2cf6c057484f2b0f9f763448151567fbab5e767b84
DEPS="libtommath"
BUILDSYS=custom
# makefile.shared needs a libtool that knows the QNX cross compiler; the
# plain makefile.unix (static archive only) is portable make. Its objects are
# compiled position-independent and then linked into the shared library
# libtool would have made: libtomcrypt.so.1.0.1, soname libtomcrypt.so.1
# (-version-info 1:1, as in makefile_include.mk). USE_LTM/LTM_DESC builds the
# libtommath math descriptor (ltm_desc) for RSA, DSA, DH and ECC; the library
# then needs libtommath.so.1.
_ltcmake() { make -C "$B" -f makefile.unix -j6 CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" AR="nto${QNX_CPU}-ar" RANLIB="nto${QNX_CPU}-ranlib" \
  CFLAGS="-O2 -fPIC -DUSE_LTM -DLTM_DESC -I$STAGE$PREFIX/include" EXTRALIBS="-L$STAGE$PREFIX/lib -ltommath" PREFIX="$PREFIX" "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _ltcmake libtomcrypt.a && cd "$B" && \
  qcc -V$QCC_TARGET -shared -Wl,-soname,libtomcrypt.so.1 -o libtomcrypt.so.1.0.1 \
    -Wl,--whole-archive libtomcrypt.a -Wl,--no-whole-archive -L"$STAGE$PREFIX/lib" -ltommath; }
# Shared library only (as libtommath): the static archive is a build step.
do_install() { _ltcmake DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/lib/libtomcrypt.a" && \
  sed -i 's,^Libs:.*,&\nLibs.private: -ltommath,; s,^Cflags:.*,Cflags: -I${includedir} -DUSE_LTM -DLTM_DESC,' "$STAGE$PREFIX/lib/pkgconfig/libtomcrypt.pc" && \
  install -m755 "$B/libtomcrypt.so.1.0.1" "$STAGE$PREFIX/lib/" && \
  ln -sf libtomcrypt.so.1.0.1 "$STAGE$PREFIX/lib/libtomcrypt.so.1" && ln -sf libtomcrypt.so.1 "$STAGE$PREFIX/lib/libtomcrypt.so"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
