# gstreamer 1.24.10

> GStreamer streaming media framework core

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/gstreamer/gstreamer.git (tag `1.24.10`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gstreamer`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gstreamer` (programs, shared libraries, run-time data) and `gstreamer-dev` (headers, pkg-config/CMake files, static libraries; needs `gstreamer`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gstreamer`](https://openqnx.github.io/oss/package/x86_64/gstreamer.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gstreamer.html)) · [`gstreamer-dev`](https://openqnx.github.io/oss/package/x86_64/gstreamer-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gstreamer-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gstreamer` (dependencies come along); one version: `apk add gstreamer=<version>` | `apk del gstreamer` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gstreamer` (dependencies first) | `oss-get remove gstreamer` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gstreamer-dev` the same way, by its own name (`apk add gstreamer-dev`, `oss-get install gstreamer-dev`, by hand with its own file and list `gstreamer-dev.list`); remove it before `gstreamer`, which it needs.

```sh
# on the QNX target, as root
apk add gstreamer gstreamer-dev
apk add qnx-test-gstreamer && /usr/local/bin/qnx-port-tests/test_gstreamer    # its on-target test
apk del gstreamer gstreamer-dev qnx-test-gstreamer
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gstreamer.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gstreamer.list
rm var/lib/manual/gstreamer.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gstreamer` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (11 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libxml2
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.24.10 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/gstreamer/gstreamer.git src/gstreamer
```

The project lives in the `subprojects/gstreamer/` subdirectory of the repository.

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gstreamer src/gstreamer/subprojects/gstreamer \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=disabled \
      -Dexamples=disabled \
      -Dbenchmarks=disabled \
      -Dtools=enabled \
      -Dgst_debug=true \
      -Dintrospection=disabled \
      -Ddoc=disabled \
      -Dptp-helper-permissions=none \
      # \
      setuid \
      helper \
      install \
      needs \
      root \
      -Dlibunwind=disabled \
      -Dlibdw=disabled \
      -Ddbghelp=disabled \
      -Dbash-completion=disabled
ninja -C build/$QNX_ARCH/gstreamer
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gstreamer install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gstreamer`.

### 5. What it installs

212 files under `/usr/local`: 15 shared-library entries, 4 programs, 123 headers.

```
/usr/local/lib/libgstbase-1.0.so
/usr/local/lib/libgstbase-1.0.so.0
/usr/local/lib/libgstbase-1.0.so.0.2410.0
/usr/local/lib/libgstcheck-1.0.so
/usr/local/lib/libgstcheck-1.0.so.0
/usr/local/lib/libgstcheck-1.0.so.0.2410.0
/usr/local/lib/libgstcontroller-1.0.so
/usr/local/lib/libgstcontroller-1.0.so.0
/usr/local/lib/libgstcontroller-1.0.so.0.2410.0
/usr/local/lib/libgstnet-1.0.so
/usr/local/lib/libgstnet-1.0.so.0
/usr/local/lib/libgstnet-1.0.so.0.2410.0
/usr/local/lib/libgstreamer-1.0.so
/usr/local/lib/libgstreamer-1.0.so.0
/usr/local/lib/libgstreamer-1.0.so.0.2410.0
/usr/local/bin/gst-inspect-1.0
...
```

The complete list is `stage/<arch>/.manifest/gstreamer` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gstreamer
ls dists/8.0/*/ | grep '^gstreamer-'
```

This writes `gstreamer-<ver>-r0.tar.gz` (run time) and `gstreamer-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gstreamer
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gstreamer-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gstreamer-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gstreamer`](../../tests/src/) links gstreamer the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gstreamer`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gstreamer
/usr/local/bin/qnx-port-tests/test_gstreamer
```

```
== gstreamer ==
  ok   ...
PASS gstreamer
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gstreamer` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gstreamer`](../../tests/src/) -- a program linked against gstreamer the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - gst_init
   - plugins were found and loaded
   - videotestsrc (gst-plugins-base)
   - videoconvert (gst-plugins-base)
   - appsink (gst-plugins-base)
   - x264enc (gst-plugins-ugly) and avdec_h264 (gst-libav)
   - x265enc (gst-plugins-bad)
   - an AV1 encoder (gst-plugins-bad)
   - openh264dec (gst-plugins-bad)
   - gst_parse_launch
   - appsink found in the pipeline
   - pipeline reached PLAYING
   - frames flowed through the pipeline
   - no error message on the bus

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | gstreamer 1.24.10-r0 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/aarch64le/gstreamer-1.24.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gstreamer 1.24.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gstreamer-1.24.10-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gstreamer 1.24.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gstreamer-1.24.10-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.24.10**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gstreamer=<version>`), sources in `ported/gstreamer/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gstreamer`; port one with `scripts/port-update gstreamer`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.24.10; GIT=https://gitlab.freedesktop.org/gstreamer/gstreamer.git; REF=1.24.10
DEPS="glib libxml2"
SUBDIR=subprojects/gstreamer
BUILDSYS=meson
MESON_ARGS_EXTRA=(
  -Dtests=disabled -Dexamples=disabled -Dbenchmarks=disabled
  -Dtools=enabled -Dgst_debug=true
  -Dintrospection=disabled -Ddoc=disabled
  -Dptp-helper-permissions=none     # setuid helper install needs root
  -Dlibunwind=disabled -Dlibdw=disabled -Ddbghelp=disabled
  -Dbash-completion=disabled
)
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
