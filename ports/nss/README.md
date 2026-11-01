# nss 3.131.1

> Mozilla Network Security Services

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.mozilla.org/pub/security/nss/releases/NSS_3_131_1_RTM/src/nss-3.131.1.tar.gz |
| SHA-256 | `b1dbc1d9e7f3f8455dfb6d58ec8f7229d8a8450c629a9e69ef2ef6081f359056` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`nspr`](../nspr/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) |
| Run-time dependencies | [`nspr`](../nspr/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_nss`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `nss` (programs, shared libraries, run-time data) and `nss-dev` (headers, pkg-config/CMake files, static libraries; needs `nss`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`nss`](https://openqnx.github.io/oss/package/x86_64/nss.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nss.html)) · [`nss-dev`](https://openqnx.github.io/oss/package/x86_64/nss-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nss-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add nss` (dependencies come along); one version: `apk add nss=<version>` | `apk del nss` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install nss` (dependencies first) | `oss-get remove nss` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`nss-dev` the same way, by its own name (`apk add nss-dev`, `oss-get install nss-dev`, by hand with its own file and list `nss-dev.list`); remove it before `nss`, which it needs.

```sh
# on the QNX target, as root
apk add nss nss-dev
apk add qnx-test-nss && /usr/local/bin/qnx-port-tests/test_nss    # its on-target test, from tests-8.0-<arch>-m-z
apk del nss nss-dev qnx-test-nss
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/nss.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/nss.list
rm var/lib/manual/nss.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nss` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild nspr sqlite zlib
```

### 2. Get the upstream source

```bash
URL=https://ftp.mozilla.org/pub/security/nss/releases/NSS_3_131_1_RTM/src/nss-3.131.1.tar.gz
mkdir -p dist src/nss
curl -fL -o dist/$(basename $URL) $URL
echo "b1dbc1d9e7f3f8455dfb6d58ec8f7229d8a8450c629a9e69ef2ef6081f359056  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/nss --strip-components=1
```

The project lives in the `nss/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/nss < ports/nss/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — QNX port

> Imported from qnx-ports/nss (NSS_3_131_1_RTM...5b70bac1b861, branch
> qnx-NSS_3_131_1_BRANCH); original authorship kept. The branch's other
> commit (60747ce8d83d, "update gtest to v1.17.0") only touches the gtests,
> which this port does not build, and is left out.
>
> gyp knows no OS "qnx": treat it like FreeBSD (BSD flock, the FREEBSD code
> paths), build freebl's AES/GHASH with -mpclmul -maes as on the other ELF
> systems, C++17 instead of C++11 (QNX's libc++ needs it), take the byte-order
> helpers from <net/netbyte.h> (QNX has no <endian.h>), and rename variables
> called "bool", which <stdbool.h> (pulled in by QNX's headers) defines as a macro.
>
> ---

Touches: `nss/cmd/multinit/multinit.c`, `nss/cmd/pwdecrypt/pwdecrypt.c`, `nss/coreconf/config.gypi`, `nss/coreconf/detect_host_arch.py`, `nss/lib/dev/ckhelper.c`, `nss/lib/freebl/freebl.gyp` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/nss/nss; B=build/$QNX_ARCH/nss; mkdir -p $B
```

```bash
do_patch() { git clone -q --depth 1 --branch "$GYP_NEXT_REF" "$GYP_NEXT_GIT" "$SRCROOT/gyp-next"; }
```

```bash
do_configure() { mkdir -p "$B/nss" && cp -a "$S/." "$B/nss/"; }
```

```bash
do_build() {
  cd "$B/nss" || return 1
  local t; case "$arch" in x86_64) t=x64;; aarch64le) t=aarch64;; esac
  # The cross-GCC driver, not qcc: gyp's ninja rules pass -MMD -MF <file>,
  # which qcc misreads as a second input file (porting-hazards 18/21).
  # CC_host: the build also compiles helpers for the build machine.
  # -DOS=qnx selects qnx-ports' gyp conditions (gyp itself would say
  # "linux"). -Dsign_libs=0: shlibsign would have to run the target's
  # softoken; its .chk files are only for FIPS mode, which stays disabled.
  CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" \
  AR="$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" CC_host=gcc CXX_host=g++ AR_host=ar \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib" \
  GYP="python3 $SRCROOT/gyp-next/gyp_main.py" \
  ./build.sh -v -j "$(nproc)" --opt -t "$t" --disable-tests \
    --with-nspr="$STAGE$PREFIX/include/nspr:$STAGE$PREFIX/lib" \
    --system-sqlite -Duse_system_zlib=1 -DOS=qnx -Dsign_libs=0
}
```

```bash
do_install() {
  local d="$B/dist" r="$B/dist/Release" v="$VERSION" nsprv
  mkdir -p "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/include/nss" "$STAGE$PREFIX/bin"
  cp -a "$r"/lib/*.so "$STAGE$PREFIX/lib/" || return 1
  cp -a "$d"/public/nss/. "$STAGE$PREFIX/include/nss/" || return 1
  # The classic NSS tools. Left out: shlibsign (FIPS .chk files only),
  # nss-config (written below), and the developer tools with generic names
  # (nss, dbtool, hw-support, validation).
  local p
  for p in certutil cmsutil crlutil modutil nssdefaults pk12util pwdecrypt signtool signver ssltap symkeyutil; do
    install -m755 "$r/bin/$p" "$STAGE$PREFIX/bin/" || return 1; done
  nsprv=$(sed -n 's/^Version: *//p' "$STAGE_SYSROOT$PREFIX/lib/pkgconfig/nspr.pc")
  sed -e "s|%prefix%|$PREFIX|; s|%exec_prefix%|\${prefix}|; s|%libdir%|\${exec_prefix}/lib|" \
      -e "s|%includedir%|\${prefix}/include/nss|; s|%NSS_VERSION%|$v|; s|%NSPR_VERSION%|$nsprv|" \
      "$S/pkg/pkg-config/nss.pc.in" > "$STAGE$PREFIX/lib/pkgconfig/nss.pc"
  sed -e "s|@prefix@|$PREFIX|; s|@MOD_MAJOR_VERSION@|${v%%.*}|" \
      -e "s|@MOD_MINOR_VERSION@|$(echo $v | cut -d. -f2)|; s|@MOD_PATCH_VERSION@|$(echo $v | cut -d. -f3)|" \
      "$S/pkg/pkg-config/nss-config.in" > "$STAGE$PREFIX/bin/nss-config"
  chmod 755 "$STAGE$PREFIX/bin/nss-config"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- NSS (Network Security Services): TLS, PKCS#11, certificates, and the certutil/pk12util/modutil tools. Built with NSS's own gyp + ninja build (build.sh), the build qnx-ports' QNX changes (patches/0001) are made for.
- gyp is not installed on the build machine: build.sh runs gyp-next, pinned here, from the source tree.
- build.sh writes out/ and ../dist next to the sources: build a copy per architecture so the two never share objects.
- build.sh has no install step: copy the libraries, public headers and tools out of dist/, and write nss.pc / nss-config for the target's paths.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nss`.

### 5. What it installs

133 files under `/usr/local`: 7 shared-library entries, 12 programs, 113 headers.

```
/usr/local/lib/libfreebl3.so
/usr/local/lib/libnss3.so
/usr/local/lib/libnssckbi.so
/usr/local/lib/libnssutil3.so
/usr/local/lib/libsmime3.so
/usr/local/lib/libsoftokn3.so
/usr/local/lib/libssl3.so
/usr/local/bin/certutil
/usr/local/bin/cmsutil
/usr/local/bin/crlutil
/usr/local/bin/modutil
/usr/local/bin/nss-config
/usr/local/bin/nssdefaults
/usr/local/bin/pk12util
/usr/local/bin/pwdecrypt
/usr/local/bin/signtool
...
```

The complete list is `stage/<arch>/.manifest/nss` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nss
ls dists/8.0/*/ | grep '^nss-'
```

This writes `nss-<ver>-r0.tar.gz` (run time) and `nss-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nss
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nss-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nss-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_nss`](../../tests/src/) links nss the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-nss`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-nss
/usr/local/bin/qnx-port-tests/test_nss
```

```
== nss ==
  ok   ...
PASS nss
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_nss` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_nss`](../../tests/src/) -- a program linked against nss the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - PK11_HashBuf SHA-256
   - SHA-256(\"abc\") known digest
   - SHA-1(\"abc\") known digest
   - internal (softoken) slot
   - HMAC-SHA256 (RFC 4231 case 2)
   - AES-128-CBC encrypt (SP 800-38A F.2.1)
   - AES-128-CBC decrypt round trip
   - EC P-256 key pair generated
   - ECDSA sign (64-byte signature)
   - ECDSA verify
   - ECDSA verify rejects another digest
   - PK11_GenerateRandom
   - NSS_VersionCheck(3.131)
   - NSS_NoDB_Init
   - NSS_Shutdown
   - mkdtemp
   - certutil -N (new database, empty password)
   - certutil -S (self-signed P-256 certificate)
   - certutil -L lists qnxtest, trust CTu,Cu,Cu (u: own key)
   - NSS_Init(sql:<dir>)
   - CERT_FindCertByNickname(qnxtest)
   - subject CN=QNX NSS Test,O=openqnx
   - issuer == subject (self-signed)
   - serial number 4242
   - signature verifies with its own key
   - ... and 2 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | nss 3.131.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/nss-3.131.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | nss 3.131.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/nss-3.131.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.131.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nss=<version>`), sources in `ported/nss/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nss`; port one with `scripts/port-update nss`.

<details><summary>recipe.sh</summary>

```bash
# NSS (Network Security Services): TLS, PKCS#11, certificates, and the
# certutil/pk12util/modutil tools. Built with NSS's own gyp + ninja build
# (build.sh), the build qnx-ports' QNX changes (patches/0001) are made for.
VERSION=3.131.1
TARBALL=https://ftp.mozilla.org/pub/security/nss/releases/NSS_3_131_1_RTM/src/nss-3.131.1.tar.gz
SHA256=b1dbc1d9e7f3f8455dfb6d58ec8f7229d8a8450c629a9e69ef2ef6081f359056
BUILDSYS=custom
SUBDIR=nss
DEPS="nspr sqlite zlib"
# gyp is not installed on the build machine: build.sh runs gyp-next, pinned
# here, from the source tree.
GYP_NEXT_GIT=https://github.com/nodejs/gyp-next.git; GYP_NEXT_REF=v0.22.4
do_patch() { git clone -q --depth 1 --branch "$GYP_NEXT_REF" "$GYP_NEXT_GIT" "$SRCROOT/gyp-next"; }
# build.sh writes out/ and ../dist next to the sources: build a copy per
# architecture so the two never share objects.
do_configure() { mkdir -p "$B/nss" && cp -a "$S/." "$B/nss/"; }
do_build() {
  cd "$B/nss" || return 1
  local t; case "$arch" in x86_64) t=x64;; aarch64le) t=aarch64;; esac
  # The cross-GCC driver, not qcc: gyp's ninja rules pass -MMD -MF <file>,
  # which qcc misreads as a second input file (porting-hazards 18/21).
  # CC_host: the build also compiles helpers for the build machine.
  # -DOS=qnx selects qnx-ports' gyp conditions (gyp itself would say
  # "linux"). -Dsign_libs=0: shlibsign would have to run the target's
  # softoken; its .chk files are only for FIPS mode, which stays disabled.
  CC="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" CXX="$QNX_HOST/usr/bin/$QNX_TRIPLE-g++" \
  AR="$QNX_HOST/usr/bin/$QNX_TRIPLE-ar" CC_host=gcc CXX_host=g++ AR_host=ar \
  CPPFLAGS="-D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib -Wl,-rpath-link,$STAGE$PREFIX/lib" \
  GYP="python3 $SRCROOT/gyp-next/gyp_main.py" \
  ./build.sh -v -j "$(nproc)" --opt -t "$t" --disable-tests \
    --with-nspr="$STAGE$PREFIX/include/nspr:$STAGE$PREFIX/lib" \
    --system-sqlite -Duse_system_zlib=1 -DOS=qnx -Dsign_libs=0
}
# build.sh has no install step: copy the libraries, public headers and tools
# out of dist/, and write nss.pc / nss-config for the target's paths.
do_install() {
  local d="$B/dist" r="$B/dist/Release" v="$VERSION" nsprv
  mkdir -p "$STAGE$PREFIX/lib/pkgconfig" "$STAGE$PREFIX/include/nss" "$STAGE$PREFIX/bin"
  cp -a "$r"/lib/*.so "$STAGE$PREFIX/lib/" || return 1
  cp -a "$d"/public/nss/. "$STAGE$PREFIX/include/nss/" || return 1
  # The classic NSS tools. Left out: shlibsign (FIPS .chk files only),
  # nss-config (written below), and the developer tools with generic names
  # (nss, dbtool, hw-support, validation).
  local p
  for p in certutil cmsutil crlutil modutil nssdefaults pk12util pwdecrypt signtool signver ssltap symkeyutil; do
    install -m755 "$r/bin/$p" "$STAGE$PREFIX/bin/" || return 1; done
  nsprv=$(sed -n 's/^Version: *//p' "$STAGE_SYSROOT$PREFIX/lib/pkgconfig/nspr.pc")
  sed -e "s|%prefix%|$PREFIX|; s|%exec_prefix%|\${prefix}|; s|%libdir%|\${exec_prefix}/lib|" \
      -e "s|%includedir%|\${prefix}/include/nss|; s|%NSS_VERSION%|$v|; s|%NSPR_VERSION%|$nsprv|" \
      "$S/pkg/pkg-config/nss.pc.in" > "$STAGE$PREFIX/lib/pkgconfig/nss.pc"
  sed -e "s|@prefix@|$PREFIX|; s|@MOD_MAJOR_VERSION@|${v%%.*}|" \
      -e "s|@MOD_MINOR_VERSION@|$(echo $v | cut -d. -f2)|; s|@MOD_PATCH_VERSION@|$(echo $v | cut -d. -f3)|" \
      "$S/pkg/pkg-config/nss-config.in" > "$STAGE$PREFIX/bin/nss-config"
  chmod 755 "$STAGE$PREFIX/bin/nss-config"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
