# libnotify 0.8.3

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libnotify.git (tag `0.8.3`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gdk-pixbuf`](../gdk-pixbuf/) |
| Run-time dependencies | [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libnotify`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libnotify
apk add libnotify-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libnotify && /usr/local/bin/qnx-port-tests/test_libnotify    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libnotify`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libnotify` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gdk-pixbuf
```

<details><summary>Full build chain (12)</summary>

[`bzip2`](../bzip2/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.8.3 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libnotify.git src/libnotify
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libnotify src/libnotify \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Dintrospection=disabled \
      -Dman=false \
      -Dgtk_doc=false \
      -Ddocbook_docs=disabled
ninja -C build/$QNX_ARCH/libnotify
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libnotify install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libnotify`.

### 5. What it installs

9 files under `/usr/local`: 3 shared-library entries, 1 programs, 4 headers.

```
/usr/local/lib/libnotify.so
/usr/local/lib/libnotify.so.4
/usr/local/lib/libnotify.so.4.0.0
/usr/local/bin/notify-send
```

The complete list is `stage/<arch>/.manifest/libnotify` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libnotify
ls dists/8.0/*/ | grep '^libnotify-'
```

This writes `libnotify-<ver>-r0.tar.gz` (run time) and `libnotify-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libnotify
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libnotify-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libnotify-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libnotify`](../../tests/src/) links libnotify the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libnotify`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libnotify
/usr/local/bin/qnx-port-tests/test_libnotify
```

```
== libnotify ==
  ok   ...
PASS libnotify
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libnotify` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libnotify`](../../tests/src/) -- a program linked against libnotify the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - notify_init
   - notify_is_initted
   - app name
   - notify_notification_new
   - notify_notification_update
   - the absence of a daemon is reported as an error
   - notify_uninit

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libnotify 0.8.3-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.8.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libnotify=<version>`), sources in `ported/libnotify/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libnotify`; port one with `scripts/port-update libnotify`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.8.3; GIT=https://gitlab.gnome.org/GNOME/libnotify.git; REF=0.8.3
DEPS="glib gdk-pixbuf"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Dintrospection=disabled -Dman=false -Dgtk_doc=false
                   -Ddocbook_docs=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
