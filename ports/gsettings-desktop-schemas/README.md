# gsettings-desktop-schemas 49.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git (tag `49.0`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add gsettings-desktop-schemas
apk add gsettings-desktop-schemas-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install gsettings-desktop-schemas`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gsettings-desktop-schemas` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 49.0 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git src/gsettings-desktop-schemas
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/gsettings-desktop-schemas src/gsettings-desktop-schemas \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dintrospection=false
ninja -C build/$QNX_ARCH/gsettings-desktop-schemas
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/gsettings-desktop-schemas install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GSettings schemas shared by GNOME/GTK applications (data only).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gsettings-desktop-schemas`.

### 5. What it installs

98 files under `/usr/local`, 1 headers.

The complete list is `stage/<arch>/.manifest/gsettings-desktop-schemas` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gsettings-desktop-schemas
ls dists/8.0/*/ | grep '^gsettings-desktop-schemas-'
```

This writes `gsettings-desktop-schemas-<ver>-r0.tar.gz` (run time) and `gsettings-desktop-schemas-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gsettings-desktop-schemas
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gsettings-desktop-schemas-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gsettings-desktop-schemas-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gsettings-desktop-schemas 49.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gsettings-desktop-schemas 49.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **49.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gsettings-desktop-schemas=<version>`), sources in `ported/gsettings-desktop-schemas/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gsettings-desktop-schemas`; port one with `scripts/port-update gsettings-desktop-schemas`.

<details><summary>recipe.sh</summary>

```bash
# GSettings schemas shared by GNOME/GTK applications (data only).
VERSION=49.0; GIT=https://gitlab.gnome.org/GNOME/gsettings-desktop-schemas.git; REF=49.0
DEPS="glib"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dintrospection=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
