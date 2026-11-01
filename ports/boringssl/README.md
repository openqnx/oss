# boringssl 0.20260929.0

> Fork of OpenSSL designed to meet Google's needs (libcrypto, libssl, bssl)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/google/boringssl.git (tag `0.20260929.0`) |
| Build system | CMake |
| QNX patches | 4 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_boringssl`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `boringssl`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`boringssl`](https://openqnx.github.io/oss/package/x86_64/boringssl.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/boringssl.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add boringssl` (dependencies come along); one version: `apk add boringssl=<version>` | `apk del boringssl` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install boringssl` (dependencies first) | `oss-get remove boringssl` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-boringssl && /usr/local/bin/qnx-port-tests/test_boringssl    # its on-target test, from tests-8.0-<arch>
apk del boringssl qnx-test-boringssl
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/boringssl.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/boringssl.list
rm var/lib/manual/boringssl.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild boringssl` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 0.20260929.0 --recurse-submodules --shallow-submodules \
    https://github.com/google/boringssl.git src/boringssl
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/boringssl < ports/boringssl/patches/0001-qnx-aarch64-cpuid-from-qnx-ports.patch
patch -p1 -d src/boringssl < ports/boringssl/patches/0002-qnx-aarch64-syspage-cpu-features.patch
patch -p1 -d src/boringssl < ports/boringssl/patches/0003-qnx-entropy-dev-urandom.patch
patch -p1 -d src/boringssl < ports/boringssl/patches/0004-optional-versioned-soname.patch
```

**[`0001-qnx-aarch64-cpuid-from-qnx-ports.patch`](patches/0001-qnx-aarch64-cpuid-from-qnx-ports.patch)** — Add QNX aarch64 CPU capability setup (OPENSSL_cpuid_setup)

> Imported from qnx-ports/boringssl, branch qnx-mediapipe (commits
> cee76a8cc7ad "Add QNX aarch64 support." and cccbcd95cd98 "Add missing arm
> header.", Ethan Leir, on top of upstream c00d7ca810e9), adapted to this
> release: the file is C++ now (crypto/*.cc, bssl namespace), the sources are
> listed in gen/sources.cmake and build.json, and OPENSSL_QNX, which the
> qnx-ports file tests, is defined from __QNX__ in <openssl/target.h>.
>
> On aarch64 BoringSSL needs one OPENSSL_cpuid_setup() per OS (it reads the
> CPU features from getauxval, sysctl, ...); without one for QNX the library
> does not link. QNX's version reports Advanced SIMD, which every AArch64 core
> has.

Touches: `build.json`, `crypto/cpu_aarch64_qnx.cc`, `gen/sources.cmake`, `include/openssl/target.h` · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-aarch64-syspage-cpu-features.patch`](patches/0002-qnx-aarch64-syspage-cpu-features.patch)** — Read the Armv8 crypto extensions from the QNX system page

> qnx-ports' OPENSSL_cpuid_setup() for QNX (patch 0001) reports NEON only,
> so on cores with the crypto extensions AES, GHASH (PMULL) and SHA ran in
> the slower NEON/generic code. QNX's startup records these features in
> the system page (SYSPAGE_ENTRY(cpuinfo)->flags, AARCH64_CPU_FLAG_AES,
> _PMULL, _SHA1, _SHA256, _SHA512, _SHA3), readable by every process without
> a kernel call: map them onto BoringSSL's ARMV8_* capability bits.

Touches: `crypto/cpu_aarch64_qnx.cc` · Upstream status: QNX-specific

**[`0003-qnx-entropy-dev-urandom.patch`](patches/0003-qnx-entropy-dev-urandom.patch)** — Draw entropy from /dev/urandom on QNX (no getentropy)

> BoringSSL's default system entropy source for systems other than Linux,
> Windows, Apple and Trusty is getentropy() from <unistd.h>; QNX 8.0 has no
> getentropy() and no getrandom(). The Linux /dev/urandom backend
> (urandom.cc) is tied to <linux/random.h> and the getrandom syscall. Give the
> getentropy backend a local getentropy() that reads QNX's /dev/urandom
> (the random manager's pool; it does not block once the system is up),
> retrying on EINTR and short reads.

Touches: `crypto/rand/getentropy.cc` · Upstream status: QNX-specific

**[`0004-optional-versioned-soname.patch`](patches/0004-optional-versioned-soname.patch)** — Optional versioned soname for libcrypto and libssl (BORINGSSL_SOVERSION)

> A shared BoringSSL build names its libraries libcrypto.so and libssl.so
> with no version: their soname, and so every consumer's NEEDED entry, is
> the bare development name. On a system that also has OpenSSL (here the
> openssl component, /usr/local/lib/libcrypto.so -> libcrypto.so.3), and on
> QNX, whose loader searches LD_LIBRARY_PATH before a program's run path,
> a BoringSSL program can then load OpenSSL's libcrypto and fail or crash.
> With -DBORINGSSL_SOVERSION=<v> the libraries are libcrypto.so.<v> and
> libssl.so.<v>, a name only BoringSSL's prefix holds. BoringSSL makes no
> ABI promise between releases, so the recipe passes the release version.

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/boringssl; B=build/$QNX_ARCH/boringssl; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  # The command in bin/, under a name that does not suggest OpenSSL's.
  mkdir -p "$STAGE$PREFIX/bin" && ln -sf ../opt/boringssl/bin/bssl "$STAGE$PREFIX/bin/bssl"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- BoringSSL: Google's fork of OpenSSL (libcrypto, libssl, the bssl tool), at a release tag, with QNX's aarch64 CPU setup (patches/0001, from github.com/qnx-ports/boringssl, branch qnx-mediapipe) and the QNX changes around it (patches/0002-0004).
- It installs libcrypto/libssl and <openssl/*.h> under the same names as the openssl component, with a different API and ABI: it gets a prefix of its own, /usr/local/opt/boringssl, and a program uses it on purpose (-I/-L.../opt/boringssl/{include,lib}). The libraries get the release as their soname (libcrypto.so.0.20260929.0, patches/0004), so a BoringSSL program never loads OpenSSL's libcrypto.so found first on LD_LIBRARY_PATH (QNX's loader takes it before the run path); bssl's run path is that prefix's lib/, absolute: QNX's loader does not expand $ORIGIN.
- BUILD_TESTING=OFF: the unit tests need Go to run and googletest to build; the run-level test checks the library on the target.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH boringssl`.

### 5. What it installs

114 files under `/usr/local`, 1 programs.

```
/usr/local/bin/bssl
```

The complete list is `stage/<arch>/.manifest/boringssl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa boringssl
ls dists/8.0/*/ | grep '^boringssl-'
```

This writes `boringssl-<ver>-r0.tar.gz` (run time) and `boringssl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add boringssl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/boringssl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/boringssl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_boringssl`](../../tests/src/) links boringssl the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-boringssl`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-boringssl
/usr/local/bin/qnx-port-tests/test_boringssl
```

```
== boringssl ==
  ok   ...
PASS boringssl
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_boringssl` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_boringssl`](../../tests/src/) -- a program linked against boringssl the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - headers are BoringSSL's (OPENSSL_IS_BORINGSSL)
   - SHA-256(\"abc\")
   - HMAC-SHA-256 (RFC 4231 case 2)
   - AES-128-GCM seal: ciphertext and tag (GCM test case 2)
   - AES-128-GCM open
   - AES-128-GCM open refuses a tampered tag
   - X25519 public key (RFC 7748 6.1, Alice)
   - X25519 shared secret (RFC 7748 6.1)
   - Ed25519 public key (RFC 8032 test 1)
   - Ed25519 signature (RFC 8032 test 1)
   - Ed25519 verify
   - Ed25519 verify refuses a modified signature
   - RAND_bytes: two different non-zero draws
   - self-signed P-256 certificate created
   - contexts and socketpair set up
   - TLS handshake completed
   - negotiated TLS 1.3
   - client verified the server certificate against its store
   - server saw SNI \"localhost\"
   - client -> server application data
   - server -> client application data

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | boringssl 0.20260929.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/boringssl-0.20260929.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | boringssl 0.20260929.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/boringssl-0.20260929.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.20260929.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add boringssl=<version>`), sources in `ported/boringssl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates boringssl`; port one with `scripts/port-update boringssl`.

<details><summary>recipe.sh</summary>

```bash
# BoringSSL: Google's fork of OpenSSL (libcrypto, libssl, the bssl tool),
# at a release tag, with QNX's aarch64 CPU setup (patches/0001, from
# github.com/qnx-ports/boringssl, branch qnx-mediapipe) and the QNX changes
# around it (patches/0002-0004).
VERSION=0.20260929.0; GIT=https://github.com/google/boringssl.git; REF=0.20260929.0
DEPS=""
BUILDSYS=cmake
# It installs libcrypto/libssl and <openssl/*.h> under the same names as the
# openssl component, with a different API and ABI: it gets a prefix of its
# own, /usr/local/opt/boringssl, and a program uses it on purpose
# (-I/-L.../opt/boringssl/{include,lib}). The libraries get the release as
# their soname (libcrypto.so.0.20260929.0, patches/0004), so a BoringSSL
# program never loads OpenSSL's libcrypto.so found first on LD_LIBRARY_PATH
# (QNX's loader takes it before the run path); bssl's run path is that
# prefix's lib/, absolute: QNX's loader does not expand $ORIGIN.
OPT="$PREFIX/opt/boringssl"
# BUILD_TESTING=OFF: the unit tests need Go to run and googletest to build;
# the run-level test checks the library on the target.
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON
                   -DBUILD_SHARED_LIBS=ON -DBORINGSSL_SOVERSION="$VERSION" -DBUILD_TESTING=OFF )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  # The command in bin/, under a name that does not suggest OpenSSL's.
  mkdir -p "$STAGE$PREFIX/bin" && ln -sf ../opt/boringssl/bin/bssl "$STAGE$PREFIX/bin/bssl"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
