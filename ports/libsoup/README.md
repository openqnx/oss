# libsoup 3.6.1

> GLib-based HTTP client and server library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libsoup.git (tag `3.6.1`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libxml2`](../libxml2/) [`sqlite`](../sqlite/) [`nghttp2`](../nghttp2/) [`libpsl`](../libpsl/) [`brotli`](../brotli/) |
| Run-time dependencies | [`brotli`](../brotli/) [`gettext`](../gettext/) [`glib`](../glib/) [`libpsl`](../libpsl/) [`nghttp2`](../nghttp2/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libsoup`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libsoup` (programs, shared libraries, run-time data) and `libsoup-dev` (headers, pkg-config/CMake files, static libraries; needs `libsoup`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libsoup`](https://openqnx.github.io/oss/package/x86_64/libsoup.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsoup.html)) · [`libsoup-dev`](https://openqnx.github.io/oss/package/x86_64/libsoup-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libsoup-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libsoup` (dependencies come along); one version: `apk add libsoup=<version>` | `apk del libsoup` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libsoup` (dependencies first) | `oss-get remove libsoup` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libsoup-dev` the same way, by its own name (`apk add libsoup-dev`, `oss-get install libsoup-dev`, by hand with its own file and list `libsoup-dev.list`); remove it before `libsoup`, which it needs.

```sh
# on the QNX target, as root
apk add libsoup libsoup-dev
apk add qnx-test-libsoup && /usr/local/bin/qnx-port-tests/test_libsoup    # its on-target test, from tests-8.0-<arch>
apk del libsoup libsoup-dev qnx-test-libsoup
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libsoup.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libsoup.list
rm var/lib/manual/libsoup.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libsoup` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (22 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libxml2 sqlite nghttp2 libpsl brotli
```

<details><summary>Full build chain (22)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libpsl`](../libpsl/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.6.1 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libsoup.git src/libsoup
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libsoup src/libsoup \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dintrospection=disabled \
      -Dvapi=disabled \
      -Ddocs=disabled \
      -Dsysprof=disabled \
      -Dgssapi=disabled \
      -Dkrb5_config= \
      -Dntlm=disabled \
      -Dbrotli=enabled \
      -Dtls_check=false
ninja -C build/$QNX_ARCH/libsoup
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libsoup install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): brotli

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libsoup`.

### 5. What it installs

116 files under `/usr/local`: 3 shared-library entries, 42 headers.

```
/usr/local/lib/libsoup-3.0.so
/usr/local/lib/libsoup-3.0.so.0
/usr/local/lib/libsoup-3.0.so.0.7.1
```

The complete list is `stage/<arch>/.manifest/libsoup` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libsoup
ls dists/8.0/*/ | grep '^libsoup-'
```

This writes `libsoup-<ver>-r0.tar.gz` (run time) and `libsoup-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libsoup
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libsoup-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libsoup-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libsoup`](../../tests/src/) links libsoup the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libsoup`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libsoup
/usr/local/bin/qnx-port-tests/test_libsoup
```

```
== libsoup ==
  ok   ...
PASS libsoup
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libsoup` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libsoup`](../../tests/src/) -- a program linked against libsoup the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - soup_server_new
   - soup_server_listen_local and a listening URI
   - soup_message_new
   - soup_session_send_and_read
   - HTTP 200
   - response body matches
   - HTTP 404 for an unknown path

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libsoup 3.6.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libsoup-3.6.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libsoup 3.6.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libsoup-3.6.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.6.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libsoup=<version>`), sources in `ported/libsoup/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libsoup`; port one with `scripts/port-update libsoup`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.6.1; GIT=https://gitlab.gnome.org/GNOME/libsoup.git; REF=3.6.1
# Also linked at run time (found by scripts/gen-dependency-index): brotli
DEPS="glib libxml2 sqlite nghttp2 libpsl brotli"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dintrospection=disabled -Dvapi=disabled
                   -Ddocs=disabled -Dsysprof=disabled -Dgssapi=disabled
                   -Dkrb5_config= -Dntlm=disabled -Dbrotli=enabled -Dtls_check=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
