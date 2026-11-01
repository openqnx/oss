# gst-plugins-ugly 1.24.10

> Quality GStreamer plugins with possible licensing issues (ugly set)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | see recipe.sh |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`gst-plugins-base`](../gst-plugins-base/) [`x264`](../x264/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`x264`](../x264/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gst_plugins_ugly`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gst-plugins-ugly` (programs, shared libraries, run-time data) and `gst-plugins-ugly-dev` (headers, pkg-config/CMake files, static libraries; needs `gst-plugins-ugly`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gst-plugins-ugly`](https://openqnx.github.io/oss/package/x86_64/gst-plugins-ugly.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gst-plugins-ugly.html)) · [`gst-plugins-ugly-dev`](https://openqnx.github.io/oss/package/x86_64/gst-plugins-ugly-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gst-plugins-ugly-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gst-plugins-ugly` (dependencies come along); one version: `apk add gst-plugins-ugly=<version>` | `apk del gst-plugins-ugly` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gst-plugins-ugly` (dependencies first) | `oss-get remove gst-plugins-ugly` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gst-plugins-ugly-dev` the same way, by its own name (`apk add gst-plugins-ugly-dev`, `oss-get install gst-plugins-ugly-dev`, by hand with its own file and list `gst-plugins-ugly-dev.list`); remove it before `gst-plugins-ugly`, which it needs.

```sh
# on the QNX target, as root
apk add gst-plugins-ugly gst-plugins-ugly-dev
apk add qnx-test-gst-plugins-ugly && /usr/local/bin/qnx-port-tests/test_gst_plugins_ugly    # its on-target test, from tests-8.0-<arch>
apk del gst-plugins-ugly gst-plugins-ugly-dev qnx-test-gst-plugins-ugly
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gst-plugins-ugly.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gst-plugins-ugly.list
rm var/lib/manual/gst-plugins-ugly.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gst-plugins-ugly` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (26 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gst-plugins-base x264
```

<details><summary>Full build chain (26)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`gst-plugins-base`](../gst-plugins-base/) [`gstreamer`](../gstreamer/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libogg`](../libogg/) [`libtheora`](../libtheora/) [`libunistring`](../libunistring/) [`libvorbis`](../libvorbis/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`opus`](../opus/) [`orc`](../orc/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`x264`](../x264/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

Fetched by the recipe's `do_fetch` hook:

The project lives in the `subprojects/gst-plugins-ugly/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gst-plugins-ugly src/gst-plugins-ugly/subprojects/gst-plugins-ugly \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Ddoc=disabled \
      -Dnls=enabled \
      -Dgpl=enabled \
      # \
      x264 \
      is \
      GPL; \
      the \
      plugin \
      set \
      refuses \
      to \
      build \
      it \
      otherwise \
      -Dx264=enabled \
      # \
      not \
      ported: \
      the \
      DVD/legacy-codec \
      corner \
      of \
      this \
      plugin \
      set \
      -Da52dec=disabled \
      -Dcdio=disabled \
      -Ddvdread=disabled \
      -Dmpeg2dec=disabled \
      -Dsidplay=disabled
ninja -C build/$QNX_ARCH/gst-plugins-ugly
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gst-plugins-ugly install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gst-plugins-ugly`.

### 5. What it installs

61 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/gst-plugins-ugly` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gst-plugins-ugly
ls dists/8.0/*/ | grep '^gst-plugins-ugly-'
```

This writes `gst-plugins-ugly-<ver>-r0.tar.gz` (run time) and `gst-plugins-ugly-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gst-plugins-ugly
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gst-plugins-ugly-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gst-plugins-ugly-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gst_plugins_ugly`](../../tests/src/) links gst-plugins-ugly the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gst-plugins-ugly`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gst-plugins-ugly
/usr/local/bin/qnx-port-tests/test_gst_plugins_ugly
```

```
== gst-plugins-ugly ==
  ok   ...
PASS gst-plugins-ugly
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gst_plugins_ugly` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gst_plugins_ugly`](../../tests/src/) -- a program linked against gst-plugins-ugly the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - x264enc (x264)
   - asf, realmedia, dvdlpcmdec, dvdsub
   - 10 access units, EOS
   - byte stream begins with a start code
   - first access unit has SPS, PPS and an IDR slice
   - an H.264 decoder is available
   - x264 stream decodes to 10 frames of 160x120

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-plugins-ugly 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gst-plugins-ugly-1.24.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gst-plugins-ugly 1.24.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gst-plugins-ugly-1.24.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-plugins-ugly 1.24.10-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gst-plugins-ugly-1.24.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gst-plugins-ugly 1.24.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gst-plugins-ugly-1.24.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gst-plugins-ugly=<version>`), sources in `ported/gst-plugins-ugly/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gst-plugins-ugly`; port one with `scripts/port-update gst-plugins-ugly`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10
SRCROOT="$SRC_DIR/gstreamer"
SUBDIR=subprojects/gst-plugins-ugly
DEPS="gst-plugins-base x264"
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Ddoc=disabled -Dnls=enabled
  -Dgpl=enabled                 # x264 is GPL; the plugin set refuses to build it otherwise
  -Dx264=enabled
  # not ported: the DVD/legacy-codec corner of this plugin set
  -Da52dec=disabled -Dcdio=disabled
  -Ddvdread=disabled -Dmpeg2dec=disabled -Dsidplay=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
