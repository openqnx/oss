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

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libsoup
apk add libsoup-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libsoup && /usr/local/bin/qnx-port-tests/test_libsoup    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libsoup`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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

**Build the dependencies first** (17 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libxml2 sqlite nghttp2 libpsl brotli
```

<details><summary>Full build chain (17)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libpsl`](../libpsl/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/)
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

[`test_libsoup`](../../tests/src/) links libsoup the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libsoup`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

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
