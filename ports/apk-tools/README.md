# apk-tools 3.0.0_rc5

> Alpine Package Keeper (apk), the Alpine package manager

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/alpinelinux/apk-tools.git (tag `v3.0.0_rc5`) |
| Build system | Meson |
| QNX patches | 4 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_apk_tools`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `apk-tools` (programs, shared libraries, run-time data) and `apk-tools-dev` (headers, pkg-config/CMake files, static libraries; needs `apk-tools`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`apk-tools`](https://openqnx.github.io/oss/package/x86_64/apk-tools.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/apk-tools.html)) · [`apk-tools-dev`](https://openqnx.github.io/oss/package/x86_64/apk-tools-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/apk-tools-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add apk-tools` (dependencies come along); one version: `apk add apk-tools=<version>` | `apk del apk-tools` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install apk-tools` (dependencies first) | `oss-get remove apk-tools` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`apk-tools-dev` the same way, by its own name (`apk add apk-tools-dev`, `oss-get install apk-tools-dev`, by hand with its own file and list `apk-tools-dev.list`); remove it before `apk-tools`, which it needs.

```sh
# on the QNX target, as root
apk add apk-tools apk-tools-dev
apk add qnx-test-apk-tools && /usr/local/bin/qnx-port-tests/test_apk_tools    # its on-target test, from tests-8.0-<arch>
apk del apk-tools apk-tools-dev qnx-test-apk-tools
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/apk-tools.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/apk-tools.list
rm var/lib/manual/apk-tools.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild apk-tools` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild openssl zlib zstd
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v3.0.0_rc5 --recurse-submodules --shallow-submodules \
    https://github.com/alpinelinux/apk-tools.git src/apk-tools
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0001-aports-qnx-compat.patch
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0002-portability-optional-sys-random-h.patch
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0003-qnx-openat-layer-dir-o-directory.patch
patch -p1 -d src/apk-tools < ports/apk-tools/patches/0004-qnx-mkpkg-subdir-o-directory.patch
```

**[`0001-aports-qnx-compat.patch`](patches/0001-aports-qnx-compat.patch)** — QNX compatibility for apk-tools (from QNX's aports core/apk-tools).

> Taken from qnx-ports/aports core/apk-tools/0001-qnx-compat.patch, the patch
> the QNX Everywhere image's apk is built with.
>
>
> qnx: compat
>
> ---
>  libfetch/common.c | 4 ++++
>  src/apk_xattr.h   | 8 ++++++++
>  src/io.c          | 6 +++---
>  3 files changed, 15 insertions(+), 3 deletions(-)

Touches: `libfetch/common.c`, `src/apk_xattr.h`, `src/io.c` · Upstream status: QNX-specific

**[`0002-portability-optional-sys-random-h.patch`](patches/0002-portability-optional-sys-random-h.patch)** — Do not require a system <sys/random.h>.

> The portability wrapper always does #include_next <sys/random.h>, but its
> whole purpose is systems without getrandom(): QNX 8 has neither the header
> nor the function, and the build stops before the bundled getrandom.c (which
> reads /dev/urandom) is used. Include the system header only if it exists.

Touches: `portability/sys/random.h` · Upstream status: QNX-specific

**[`0003-qnx-openat-layer-dir-o-directory.patch`](patches/0003-qnx-openat-layer-dir-o-directory.patch)** — Open the database layer directory with O_DIRECTORY.

> apk_db_read_layer() opens lib/apk/db with plain O_RDONLY and then reads
> installed, triggers and scripts.tar from it with openat(). On QNX 8 a
> directory descriptor is only accepted as an openat() base if it was opened
> with O_DIRECTORY; otherwise every openat() under it fails with ENOTDIR
> (verified on the target). The first "apk --initdb add" works because the
> directory does not exist yet, but every later apk command on that root stops
> with "Unable to read database: Not a directory". Open the layer directory
> with O_DIRECTORY, as apk already does for the root and cache directories.

Touches: `src/database.c` · Upstream status: QNX-specific

**[`0004-qnx-mkpkg-subdir-o-directory.patch`](patches/0004-qnx-mkpkg-subdir-o-directory.patch)** — Open mkpkg's subdirectories with O_DIRECTORY.

> mkpkg_process_dirent() opens each subdirectory of the -F tree with plain
> O_RDONLY and then walks it with fstatat()/openat(). On QNX 8 a directory
> descriptor is only accepted as an *at() base if it was opened with
> O_DIRECTORY (see 0003), so every package with a nested directory failed:
> "failed to process directory 'usr': -1" and "failed to create package:
> Operation not permitted". The top-level -F directory was already opened
> with O_DIRECTORY, which is why a flat file tree worked.

Touches: `src/app_mkpkg.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/apk-tools src/apk-tools \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Ddocs=disabled \
      -Dhelp=disabled \
      -Dlua=disabled \
      -Dpython=disabled \
      -Dtests=disabled \
      -Dcrypto_backend=openssl \
      -Dzstd=true \
      -Dc_args=-D_QNX_SOURCE \
      -Dc_link_args=-lsocket
ninja -C build/$QNX_ARCH/apk-tools
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/apk-tools install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- apk-tools, the Alpine package manager the QNX Everywhere image (and repo.oss.qnx.com) uses -- with it a target can install this repository's APK packages. Version as in the image.
- Not linked, but needed to fetch from an https:// repository (the GitHub releases): the CA bundle at /usr/local/ssl/cert.pem, OpenSSL's default.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH apk-tools`.

### 5. What it installs

31 files under `/usr/local`: 2 shared-library entries, 1 programs, 26 headers.

```
/usr/local/lib/libapk.so
/usr/local/lib/libapk.so.3.0.0_rc5
/usr/local/bin/apk
```

The complete list is `stage/<arch>/.manifest/apk-tools` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa apk-tools
ls dists/8.0/*/ | grep '^apk-tools-'
```

This writes `apk-tools-<ver>-r0.tar.gz` (run time) and `apk-tools-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add apk-tools
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/apk-tools-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/apk-tools-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_apk_tools`](../../tests/src/) links apk-tools the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-apk-tools`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-apk-tools
/usr/local/bin/qnx-port-tests/test_apk_tools
```

```
== apk-tools ==
  ok   ...
PASS apk-tools
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_apk_tools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_apk_tools`](../../tests/src/) -- a program linked against apk-tools the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | apk-tools 3.0.0_rc5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/apk-tools-3.0.0_rc5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | apk-tools 3.0.0_rc5-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/apk-tools-3.0.0_rc5-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | apk-tools 3.0.0_rc5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/apk-tools-3.0.0_rc5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | apk-tools 3.0.0_rc5-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/apk-tools-3.0.0_rc5-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.0.0_rc5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add apk-tools=<version>`), sources in `ported/apk-tools/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates apk-tools`; port one with `scripts/port-update apk-tools`.

<details><summary>recipe.sh</summary>

```bash
# apk-tools, the Alpine package manager the QNX Everywhere image (and
# repo.oss.qnx.com) uses -- with it a target can install this repository's APK
# packages. Version as in the image.
VERSION=3.0.0_rc5; GIT=https://github.com/alpinelinux/apk-tools.git; REF=v3.0.0_rc5
DEPS="openssl zlib zstd"
# Not linked, but needed to fetch from an https:// repository (the GitHub
# releases): the CA bundle at /usr/local/ssl/cert.pem, OpenSSL's default.
RUNTIME_DEPS="ca-certificates"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Ddocs=disabled -Dhelp=disabled -Dlua=disabled -Dpython=disabled -Dtests=disabled -Dcrypto_backend=openssl -Dzstd=true -Dc_args=-D_QNX_SOURCE -Dc_link_args=-lsocket )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
