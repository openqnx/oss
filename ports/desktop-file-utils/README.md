# desktop-file-utils 0.28

> Command line utilities for validating and installing .desktop files

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xdg/desktop-file-utils.git (tag `0.28`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_desktop_file_utils`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add desktop-file-utils
apk add desktop-file-utils-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-desktop-file-utils && /usr/local/bin/qnx-port-tests/test_desktop_file_utils    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install desktop-file-utils`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild desktop-file-utils` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.28 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/xdg/desktop-file-utils.git src/desktop-file-utils
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/desktop-file-utils src/desktop-file-utils \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both
ninja -C build/$QNX_ARCH/desktop-file-utils
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/desktop-file-utils install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- desktop-file-utils: validate and install .desktop files, update-desktop-database.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH desktop-file-utils`.

### 5. What it installs

9 files under `/usr/local`, 4 programs.

```
/usr/local/bin/desktop-file-edit
/usr/local/bin/desktop-file-install
/usr/local/bin/desktop-file-validate
/usr/local/bin/update-desktop-database
```

The complete list is `stage/<arch>/.manifest/desktop-file-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa desktop-file-utils
ls dists/8.0/*/ | grep '^desktop-file-utils-'
```

This writes `desktop-file-utils-<ver>-r0.tar.gz` (run time) and `desktop-file-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add desktop-file-utils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/desktop-file-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/desktop-file-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_desktop_file_utils`](../../tests/src/) links desktop-file-utils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-desktop-file-utils`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-desktop-file-utils
/usr/local/bin/qnx-port-tests/test_desktop_file_utils
```

```
== desktop-file-utils ==
  ok   ...
PASS desktop-file-utils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_desktop_file_utils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_desktop_file_utils`](../../tests/src/) -- a program linked against desktop-file-utils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | desktop-file-utils 0.28-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/desktop-file-utils-0.28-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | desktop-file-utils 0.28-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/desktop-file-utils-0.28-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.28**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add desktop-file-utils=<version>`), sources in `ported/desktop-file-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates desktop-file-utils`; port one with `scripts/port-update desktop-file-utils`.

<details><summary>recipe.sh</summary>

```bash
# desktop-file-utils: validate and install .desktop files, update-desktop-database.
VERSION=0.28; GIT=https://gitlab.freedesktop.org/xdg/desktop-file-utils.git; REF=0.28
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
