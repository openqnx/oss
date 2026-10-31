# lxde-icon-theme 0.5.2

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/lxde/lxde-icon-theme/archive/refs/tags/0.5.2.tar.gz |
| SHA-256 | `5b71da247ba25ddcd991a3a184ca5ac92f40b7676766e1e59437067a20f7ecf7` |
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
apk add lxde-icon-theme
apk add lxde-icon-theme-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install lxde-icon-theme`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild lxde-icon-theme` performs exactly the same steps (2–5) in one command.

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
URL=https://github.com/lxde/lxde-icon-theme/archive/refs/tags/0.5.2.tar.gz
mkdir -p dist src/lxde-icon-theme
curl -fL -o dist/$(basename $URL) $URL
echo "5b71da247ba25ddcd991a3a184ca5ac92f40b7676766e1e59437067a20f7ecf7  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/lxde-icon-theme --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/lxde-icon-theme < ports/lxde-icon-theme/patches/0001-aports-qnx-fix.patch
```

**[`0001-aports-qnx-fix.patch`](patches/0001-aports-qnx-fix.patch)** — QNX change from QNX's aports (extra/lxde-icon-theme/qnx-fix.patch).

> Taken from qnx-ports/aports extra/lxde-icon-theme -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `lxde-icon-theme-0.5.2/Makefile.am` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/lxde-icon-theme; B=build/$QNX_ARCH/lxde-icon-theme; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && ./configure --prefix="$PREFIX"; }
```

```bash
do_build() { make -C "$B"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The LXDE (nuoveXT) icon theme (data only). Version as in the QNX Everywhere 8.0.4r1 image.
- Icon data only, installed by its Makefiles from the source tree; build in a copy of it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH lxde-icon-theme`.

### 5. What it installs

6590 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/lxde-icon-theme` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa lxde-icon-theme
ls dists/8.0/*/ | grep '^lxde-icon-theme-'
```

This writes `lxde-icon-theme-<ver>-r0.tar.gz` (run time) and `lxde-icon-theme-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add lxde-icon-theme
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/lxde-icon-theme-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/lxde-icon-theme-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lxde-icon-theme 0.5.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/lxde-icon-theme-0.5.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lxde-icon-theme 0.5.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/lxde-icon-theme-0.5.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.5.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add lxde-icon-theme=<version>`), sources in `ported/lxde-icon-theme/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates lxde-icon-theme`; port one with `scripts/port-update lxde-icon-theme`.

<details><summary>recipe.sh</summary>

```bash
# The LXDE (nuoveXT) icon theme (data only). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=0.5.2
TARBALL=https://github.com/lxde/lxde-icon-theme/archive/refs/tags/0.5.2.tar.gz
SHA256=5b71da247ba25ddcd991a3a184ca5ac92f40b7676766e1e59437067a20f7ecf7
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# Icon data only, installed by its Makefiles from the source tree; build in a
# copy of it.
BUILDSYS=custom
do_configure() { cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && ./configure --prefix="$PREFIX"; }
do_build() { make -C "$B"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
