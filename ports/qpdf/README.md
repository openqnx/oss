# qpdf 11.10.1

> PDF Transformation System

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/qpdf/qpdf/releases/download/v11.10.1/qpdf-11.10.1.tar.gz |
| SHA-256 | `defca435cf57d26f8a0619864841aa21f5469fddc6eb5662f62d8443021c069d` |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`openssl`](../openssl/) |
| Run-time dependencies | [`libjpeg-turbo`](../libjpeg-turbo/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_qpdf`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `qpdf` (programs, shared libraries, run-time data) and `qpdf-dev` (headers, pkg-config/CMake files, static libraries; needs `qpdf`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`qpdf`](https://openqnx.github.io/oss/package/x86_64/qpdf.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qpdf.html)) · [`qpdf-dev`](https://openqnx.github.io/oss/package/x86_64/qpdf-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/qpdf-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add qpdf` (dependencies come along); one version: `apk add qpdf=<version>` | `apk del qpdf` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install qpdf` (dependencies first) | `oss-get remove qpdf` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`qpdf-dev` the same way, by its own name (`apk add qpdf-dev`, `oss-get install qpdf-dev`, by hand with its own file and list `qpdf-dev.list`); remove it before `qpdf`, which it needs.

```sh
# on the QNX target, as root
apk add qpdf qpdf-dev
apk add qnx-test-qpdf && /usr/local/bin/qnx-port-tests/test_qpdf    # its on-target test, from tests-8.0-<arch>-m-z
apk del qpdf qpdf-dev qnx-test-qpdf
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/qpdf.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/qpdf.list
rm var/lib/manual/qpdf.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild qpdf` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild zlib libjpeg-turbo openssl
```

### 2. Get the upstream source

```bash
URL=https://github.com/qpdf/qpdf/releases/download/v11.10.1/qpdf-11.10.1.tar.gz
mkdir -p dist src/qpdf
curl -fL -o dist/$(basename $URL) $URL
echo "defca435cf57d26f8a0619864841aa21f5469fddc6eb5662f62d8443021c069d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/qpdf --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/qpdf < ports/qpdf/patches/0001-crypto-provider-factory-lambda.patch
```

**[`0001-crypto-provider-factory-lambda.patch`](patches/0001-crypto-provider-factory-lambda.patch)** — Register crypto providers with a lambda, not &std::make_shared.

> QPDFCryptoProvider stores each implementation's factory as
> std::make_shared<T> taken as a function. With libc++ (QNX's C++ library)
> make_shared is an overload set (it also declares the array forms), so the
> name does not resolve to one function: "no match for 'operator=' ...
> '<unresolved overloaded function type>'". Taking the address of a standard
> library function is unspecified anyway; store a lambda that calls
> make_shared instead (qpdf 12 still does the same).

Touches: `libqpdf/QPDFCryptoProvider.cc` · Upstream status: Pending (affects any libc++ build)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/qpdf -B build/$QNX_ARCH/qpdf -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_STATIC_LIBS=OFF \
      -DBUILD_DOC=OFF \
      -DUSE_IMPLICIT_CRYPTO=OFF \
      -DREQUIRE_CRYPTO_NATIVE=ON \
      -DREQUIRE_CRYPTO_OPENSSL=ON \
      -DDEFAULT_CRYPTO=openssl \
      -DINSTALL_EXAMPLES=OFF \
      -DINSTALL_MANUAL=OFF
cmake --build build/$QNX_ARCH/qpdf -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/qpdf
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- qpdf: structural, content-preserving PDF transformation -- the library (libqpdf, C++ and C API) and the qpdf, fix-qdf and zlib-flate programs: linearise, encrypt/decrypt, split/merge pages, inspect PDF structure. 11.10 (the last 11.x), not 12: cups-filters 1.28 uses the PointerHolder and replaceOrRemoveKey() API that qpdf 12 removed.
- Crypto (RC4/AES/SHA for PDF encryption) from OpenSSL; the native implementation stays built in as well. No static library, no manual, no example sources.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH qpdf`.

### 5. What it installs

85 files under `/usr/local`: 3 shared-library entries, 3 programs, 70 headers.

```
/usr/local/lib/libqpdf.so
/usr/local/lib/libqpdf.so.29
/usr/local/lib/libqpdf.so.29.10.1
/usr/local/bin/fix-qdf
/usr/local/bin/qpdf
/usr/local/bin/zlib-flate
```

The complete list is `stage/<arch>/.manifest/qpdf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa qpdf
ls dists/8.0/*/ | grep '^qpdf-'
```

This writes `qpdf-<ver>-r0.tar.gz` (run time) and `qpdf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add qpdf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/qpdf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/qpdf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_qpdf`](../../tests/src/) links qpdf the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-qpdf`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-qpdf
/usr/local/bin/qnx-port-tests/test_qpdf
```

```
== qpdf ==
  ok   ...
PASS qpdf
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_qpdf` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_qpdf`](../../tests/src/) -- a program linked against qpdf the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - QPDF::QPDFVersion() 11.10.1
   - scratch directory
   - 3-page PDF written
   - read back: 3 pages
   - each page's decoded content stream is the text written
   - PDF version 1.3 (emptyPDF)
   - AES-256 (R6) encrypted copy written
   - read back and encrypt
   - encrypted file without a password: qpdf_e_password
   - opened with the user password: encrypted, R=6
   - decrypted page 2 content intact
   - open with the user password
   - C API: qpdf_read + qpdf_get_num_pages = 3
   - qpdf --check: no syntax or stream encoding errors
   - qpdf --show-npages: 3
   - qpdf --pages 3,1: a 2-page file
   - qpdf --show-encryption: R = 6, AESv3
   - linearized copy: no linearization errors
   - qpdf --decrypt: 3 pages
   - zlib-flate -compress | -uncompress round trip

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | qpdf 11.10.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/qpdf-11.10.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | qpdf 11.10.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/qpdf-11.10.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **11.10.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add qpdf=<version>`), sources in `ported/qpdf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates qpdf`; port one with `scripts/port-update qpdf`.

<details><summary>recipe.sh</summary>

```bash
# qpdf: structural, content-preserving PDF transformation -- the library
# (libqpdf, C++ and C API) and the qpdf, fix-qdf and zlib-flate programs:
# linearise, encrypt/decrypt, split/merge pages, inspect PDF structure.
# 11.10 (the last 11.x), not 12: cups-filters 1.28 uses the PointerHolder and
# replaceOrRemoveKey() API that qpdf 12 removed.
VERSION=11.10.1
TARBALL=https://github.com/qpdf/qpdf/releases/download/v11.10.1/qpdf-11.10.1.tar.gz
SHA256=defca435cf57d26f8a0619864841aa21f5469fddc6eb5662f62d8443021c069d
BUILDSYS=cmake
DEPS="zlib libjpeg-turbo openssl"
# Crypto (RC4/AES/SHA for PDF encryption) from OpenSSL; the native
# implementation stays built in as well. No static library, no manual, no
# example sources.
CMAKE_ARGS_EXTRA=( -DBUILD_STATIC_LIBS=OFF -DBUILD_DOC=OFF -DUSE_IMPLICIT_CRYPTO=OFF -DREQUIRE_CRYPTO_NATIVE=ON -DREQUIRE_CRYPTO_OPENSSL=ON
                   -DDEFAULT_CRYPTO=openssl -DINSTALL_EXAMPLES=OFF -DINSTALL_MANUAL=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
