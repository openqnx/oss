# openttd-opengfx 8.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://cdn.openttd.org/opengfx-releases/8.0/opengfx-8.0-all.zip |
| SHA-256 | `43a0c1dabf39cb865394f3a6cc36d4da5c10ecfaaf55652043104806810903be` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_openttd_sets`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add openttd-opengfx
apk add openttd-opengfx-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-openttd-sets && /usr/local/bin/qnx-port-tests/test_openttd_sets    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install openttd-opengfx`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild openttd-opengfx` performs exactly the same steps (2–5) in one command.

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
URL=https://cdn.openttd.org/opengfx-releases/8.0/opengfx-8.0-all.zip
mkdir -p dist src/openttd-opengfx
curl -fL -o dist/$(basename $URL) $URL
echo "43a0c1dabf39cb865394f3a6cc36d4da5c10ecfaaf55652043104806810903be  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/openttd-opengfx --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/openttd-opengfx; B=build/$QNX_ARCH/openttd-opengfx; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local d="$STAGE$PREFIX/share/games/openttd/baseset"; mkdir -p "$d"
  local t; t=$(ls "$S"/*.tar | head -n 1); [ -f "$t" ] || return 1
  tar xf "$t" -C "$d"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- openttd-opengfx: OpenGFX, the free base graphics set for OpenTTD (no original game data needed). Data only: the published archive (a .tar inside a .zip) is unpacked into share/games/openttd/baseset, where OpenTTD looks for base sets.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH openttd-opengfx`.

### 5. What it installs

10 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/openttd-opengfx` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa openttd-opengfx
ls dists/8.0/*/ | grep '^openttd-opengfx-'
```

This writes `openttd-opengfx-<ver>-r0.tar.gz` (run time) and `openttd-opengfx-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add openttd-opengfx
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/openttd-opengfx-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/openttd-opengfx-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_openttd_sets`](../../tests/src/) links openttd-opengfx the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-openttd-sets`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-openttd-sets
/usr/local/bin/qnx-port-tests/test_openttd_sets
```

```
== openttd-opengfx ==
  ok   ...
PASS openttd-opengfx
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_openttd_sets` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_openttd_sets`](../../tests/src/) -- a program linked against openttd-opengfx the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openttd-opengfx 8.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openttd-openmsx 0.4.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | openttd-opensfx 1.0.3-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **8.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add openttd-opengfx=<version>`), sources in `ported/openttd-opengfx/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates openttd-opengfx`; port one with `scripts/port-update openttd-opengfx`.

<details><summary>recipe.sh</summary>

```bash
# openttd-opengfx: OpenGFX, the free base graphics set for OpenTTD (no original game data needed).
# Data only: the published archive (a .tar inside a .zip) is unpacked into
# share/games/openttd/baseset, where OpenTTD looks for base sets.
VERSION=8.0
TARBALL=https://cdn.openttd.org/opengfx-releases/8.0/opengfx-8.0-all.zip
SHA256=43a0c1dabf39cb865394f3a6cc36d4da5c10ecfaaf55652043104806810903be
DEPS=""
RUNTIME_DEPS="openttd"
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_configure() { :; }
do_build() { :; }
do_install() {
  local d="$STAGE$PREFIX/share/games/openttd/baseset"; mkdir -p "$d"
  local t; t=$(ls "$S"/*.tar | head -n 1); [ -f "$t" ] || return 1
  tar xf "$t" -C "$d"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
