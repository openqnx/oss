# gdk-pixbuf 2.42.12

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gdk-pixbuf.git (tag `2.42.12`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libpng`](../libpng/) [`libjpeg-turbo`](../libjpeg-turbo/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gdk_pixbuf`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gdk-pixbuf
apk add gdk-pixbuf-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-gdk-pixbuf && /usr/local/bin/qnx-port-tests/test_gdk_pixbuf    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gdk-pixbuf`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gdk-pixbuf` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib libpng libjpeg-turbo
```

<details><summary>Full build chain (11)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.42.12 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gdk-pixbuf.git src/gdk-pixbuf
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gdk-pixbuf src/gdk-pixbuf \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dinstalled_tests=false \
      -Dman=false \
      -Dgtk_doc=false \
      -Dintrospection=disabled \
      -Dpng=enabled \
      -Djpeg=enabled \
      -Dtiff=disabled \
      -Dothers=enabled \
      -Dbuiltin_loaders=all \
      -Drelocatable=false \
      # \
      gio_sniffing \
      pulls \
      in \
      shared-mime-info, \
      a \
      desktop \
      data \
      # \
      package; \
      gdk-pixbuf's \
      own \
      image \
      sniffing \
      is \
      what \
      an \
      # \
      embedded \
      target \
      wants \
      anyway. \
      -Dgio_sniffing=false
ninja -C build/$QNX_ARCH/gdk-pixbuf
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gdk-pixbuf install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gdk-pixbuf`.

### 5. What it installs

129 files under `/usr/local`: 3 shared-library entries, 3 programs, 13 headers.

```
/usr/local/lib/libgdk_pixbuf-2.0.so
/usr/local/lib/libgdk_pixbuf-2.0.so.0
/usr/local/lib/libgdk_pixbuf-2.0.so.0.4200.12
/usr/local/bin/gdk-pixbuf-csource
/usr/local/bin/gdk-pixbuf-pixdata
/usr/local/bin/gdk-pixbuf-query-loaders
```

The complete list is `stage/<arch>/.manifest/gdk-pixbuf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gdk-pixbuf
ls dists/8.0/*/ | grep '^gdk-pixbuf-'
```

This writes `gdk-pixbuf-<ver>-r0.tar.gz` (run time) and `gdk-pixbuf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gdk-pixbuf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gdk-pixbuf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gdk-pixbuf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gdk_pixbuf`](../../tests/src/) links gdk-pixbuf the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gdk-pixbuf`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gdk-pixbuf
/usr/local/bin/qnx-port-tests/test_gdk_pixbuf
```

```
== gdk-pixbuf ==
  ok   ...
PASS gdk-pixbuf
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gdk_pixbuf` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gdk_pixbuf`](../../tests/src/) -- a program linked against gdk-pixbuf the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - gdk_pixbuf_new
   - pixbuf geometry
   - gdk_pixbuf_fill wrote RGBA
   - at least one image format is registered
   - decoded image has the right size
   - gdk_pixbuf_scale_simple

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gdk-pixbuf 2.42.12-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/gdk-pixbuf-2.42.12-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gdk-pixbuf 2.42.12-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/gdk-pixbuf-2.42.12-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.42.12**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gdk-pixbuf=<version>`), sources in `ported/gdk-pixbuf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gdk-pixbuf`; port one with `scripts/port-update gdk-pixbuf`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.42.12; GIT=https://gitlab.gnome.org/GNOME/gdk-pixbuf.git; REF=2.42.12
DEPS="glib libpng libjpeg-turbo"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dinstalled_tests=false -Dman=false
                   -Dgtk_doc=false -Dintrospection=disabled
                   -Dpng=enabled -Djpeg=enabled -Dtiff=disabled
                   -Dothers=enabled -Dbuiltin_loaders=all
                   -Drelocatable=false
                   # gio_sniffing pulls in shared-mime-info, a desktop data
                   # package; gdk-pixbuf's own image sniffing is what an
                   # embedded target wants anyway.
                   -Dgio_sniffing=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
