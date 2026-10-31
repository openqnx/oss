# file 5.46

> File type identification tool and libmagic library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://astron.com/pub/file/file-5.46.tar.gz |
| SHA-256 | `c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`xz`](../xz/) [`zstd`](../zstd/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_file`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `file` (programs, shared libraries, run-time data) and `file-dev` (headers, pkg-config/CMake files, static libraries; needs `file`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`file`](https://openqnx.github.io/oss/package/x86_64/file.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/file.html)) · [`file-dev`](https://openqnx.github.io/oss/package/x86_64/file-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/file-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add file` (dependencies come along); one version: `apk add file=<version>` | `apk del file` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install file` (dependencies first) | `oss-get remove file` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`file-dev` the same way, by its own name (`apk add file-dev`, `oss-get install file-dev`, by hand with its own file and list `file-dev.list`); remove it before `file`, which it needs.

```sh
# on the QNX target, as root
apk add file file-dev
apk add qnx-test-file && /usr/local/bin/qnx-port-tests/test_file    # its on-target test
apk del file file-dev qnx-test-file
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/file.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/file.list
rm var/lib/manual/file.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild file` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild zlib bzip2 xz zstd
```

<details><summary>Full build chain (8)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://astron.com/pub/file/file-5.46.tar.gz
mkdir -p dist src/file
curl -fL -o dist/$(basename $URL) $URL
echo "c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/file --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/file < ports/file/patches/0001-aports-file-5.46-pr-579-net147-fix-stack-overrun.patch
```

**[`0001-aports-file-5.46-pr-579-net147-fix-stack-overrun.patch`](patches/0001-aports-file-5.46-pr-579-net147-fix-stack-overrun.patch)** — QNX change from QNX's aports (extra/file/file-5.46-pr-579-net147-fix-stack-overrun.patch).

> Taken from qnx-ports/aports extra/file -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> PR/579: net147: Fix stack overrun.
>
> ---
>  src/readelf.c | 4 ++--
>  1 file changed, 2 insertions(+), 2 deletions(-)

Touches: `src/readelf.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/file; B=build/$QNX_ARCH/file; mkdir -p $B
```

```bash
do_build() {
  local h="$B/host-file"
  [ -x "$h/src/file" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc "$S/configure" --disable-shared --disable-libseccomp \
      --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib && _host_env make -C "$h" -j"$(nproc)"; } || return 1
  PATH="$h/src:$PATH" make -C "$B" -j"$(nproc)"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- file(1) and libmagic. Version as in the QNX Everywhere 8.0.4r1 image.
- libmagic uses regcomp()/regexec(), which QNX keeps in libregex.
- Cross-compiling the magic database needs a build-host file(1) of exactly this version; build one from the same source and put it first in PATH.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH file`.

### 5. What it installs

9 files under `/usr/local`: 2 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libmagic.so
/usr/local/lib/libmagic.so.1
/usr/local/bin/file
```

The complete list is `stage/<arch>/.manifest/file` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa file
ls dists/8.0/*/ | grep '^file-'
```

This writes `file-<ver>-r0.tar.gz` (run time) and `file-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add file
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/file-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/file-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_file`](../../tests/src/) links file the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-file`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-file
/usr/local/bin/qnx-port-tests/test_file
```

```
== file ==
  ok   ...
PASS file
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_file` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_file`](../../tests/src/) -- a program linked against file the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - magic_version() == 546 (file 5.46)
   - magic_open
   - magic_load(default database)
   - PNG header identified with size and depth
   - gzip stream identified
   - PDF 1.4 identified
   - #!/bin/sh script identified
   - plain ASCII text identified
   - own binary: ELF 64-bit LSB
   - own binary: x86-64
   - own binary: ARM aarch64
   - magic_setflags(MAGIC_MIME_TYPE)
   - MIME type of PNG is image/png
   - MIME type of gzip is application/gzip
   - MIME type of text is text/plain
   - mkdtemp
   - sample files written
   - file -v reports file-5.46
   - file: PNG
   - file: gzip
   - file --mime-type: text/plain
   - file <path>: \"<path>: ASCII text\"

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | file 5.46-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/file-5.46-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | file 5.46-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/file-5.46-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | file 5.46-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/file-5.46-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | file 5.46-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/file-5.46-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.46**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add file=<version>`), sources in `ported/file/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates file`; port one with `scripts/port-update file`.

<details><summary>recipe.sh</summary>

```bash
# file(1) and libmagic. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.46
TARBALL=https://astron.com/pub/file/file-5.46.tar.gz
SHA256=c9cc77c7c560c543135edc555af609d5619dbef011997e988ce40a3d75d86088
DEPS="zlib bzip2 xz zstd"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-libseccomp )
# libmagic uses regcomp()/regexec(), which QNX keeps in libregex.
CONF_ENV=( LIBS="-lregex" )
# Cross-compiling the magic database needs a build-host file(1) of exactly
# this version; build one from the same source and put it first in PATH.
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
do_build() {
  local h="$B/host-file"
  [ -x "$h/src/file" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc "$S/configure" --disable-shared --disable-libseccomp \
      --disable-zlib --disable-bzlib --disable-xzlib --disable-zstdlib && _host_env make -C "$h" -j"$(nproc)"; } || return 1
  PATH="$h/src:$PATH" make -C "$B" -j"$(nproc)"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
