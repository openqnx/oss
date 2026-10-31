# xdg-utils 1.2.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz |
| SHA-256 | `f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xdg-utils
apk add xdg-utils-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xdg-utils`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xdg-utils` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz
mkdir -p dist src/xdg-utils
curl -fL -o dist/$(basename $URL) $URL
echo "f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xdg-utils --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xdg-utils < ports/xdg-utils/patches/0001-aports-busybox-sed.patch
```

**[`0001-aports-busybox-sed.patch`](patches/0001-aports-busybox-sed.patch)** — QNX change from QNX's aports (extra/xdg-utils/busybox-sed.patch).

> Taken from qnx-ports/aports extra/xdg-utils -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> Busybox sed allows } to be a label so we need to add a ;.
>
> Ref https://gitlab.freedesktop.org/xdg/xdg-utils/-/merge_requests/126

Touches: `scripts/xdg-screensaver.in` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/xdg-utils; B=build/$QNX_ARCH/xdg-utils; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && ./configure --prefix="$PREFIX"; }
```

```bash
do_build() { make -C "$B/scripts" scripts XMLTO="python3 $RECIPE_DIR/xdg-utils/refentry2txt.py"; }
```

```bash
do_install() { make -C "$B/scripts" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xdg-utils: xdg-open and friends (shell scripts). Version as in the QNX Everywhere 8.0.4r1 image.
- The Makefiles only work in the source tree (the HTML/man pages are made from desc/*.xml by relative paths); build in a copy of it, scripts only.
- Each script embeds its --help/--manual text, which the Makefile makes with "xmlto txt" from desc/*.xml. xmlto (with the DocBook XSL and a text browser) is rarely on a build machine; refentry2txt.py renders the same text layout with Python alone.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xdg-utils`.

### 5. What it installs

8 files under `/usr/local`, 8 programs.

```
/usr/local/bin/xdg-desktop-icon
/usr/local/bin/xdg-desktop-menu
/usr/local/bin/xdg-email
/usr/local/bin/xdg-icon-resource
/usr/local/bin/xdg-mime
/usr/local/bin/xdg-open
/usr/local/bin/xdg-screensaver
/usr/local/bin/xdg-settings
```

The complete list is `stage/<arch>/.manifest/xdg-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xdg-utils
ls dists/8.0/*/ | grep '^xdg-utils-'
```

This writes `xdg-utils-<ver>-r0.tar.gz` (run time) and `xdg-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xdg-utils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xdg-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xdg-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xdg-utils 1.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xdg-utils-1.2.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xdg-utils 1.2.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xdg-utils-1.2.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xdg-utils=<version>`), sources in `ported/xdg-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xdg-utils`; port one with `scripts/port-update xdg-utils`.

<details><summary>recipe.sh</summary>

```bash
# xdg-utils: xdg-open and friends (shell scripts).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.2.1
TARBALL=https://gitlab.freedesktop.org/xdg/xdg-utils/-/archive/v1.2.1/xdg-utils-v1.2.1.tar.gz
SHA256=f6b648c064464c2636884c05746e80428110a576f8daacf46ef2e554dcfdae75
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# The Makefiles only work in the source tree (the HTML/man pages are made from
# desc/*.xml by relative paths); build in a copy of it, scripts only.
BUILDSYS=custom
do_configure() { cp -a "$S/." "$B/" && cd "$B" && ./configure --prefix="$PREFIX"; }
# Each script embeds its --help/--manual text, which the Makefile makes with
# "xmlto txt" from desc/*.xml. xmlto (with the DocBook XSL and a text browser)
# is rarely on a build machine; refentry2txt.py renders the same text layout
# with Python alone.
do_build() { make -C "$B/scripts" scripts XMLTO="python3 $RECIPE_DIR/xdg-utils/refentry2txt.py"; }
do_install() { make -C "$B/scripts" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
