# shared-mime-info 2.4

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/xdg/shared-mime-info/-/archive/2.4/shared-mime-info-2.4.tar.gz |
| SHA-256 | `531291d0387eb94e16e775d7e73788d06d2b2fdd8cd2ac6b6b15287593b6a2de` |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`libxml2`](../libxml2/) |
| Run-time dependencies | [`glib`](../glib/) [`libxml2`](../libxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add shared-mime-info
apk add shared-mime-info-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install shared-mime-info`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild shared-mime-info` performs exactly the same steps (2–5) in one command.

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
URL=https://gitlab.freedesktop.org/xdg/shared-mime-info/-/archive/2.4/shared-mime-info-2.4.tar.gz
mkdir -p dist src/shared-mime-info
curl -fL -o dist/$(basename $URL) $URL
echo "531291d0387eb94e16e775d7e73788d06d2b2fdd8cd2ac6b6b15287593b6a2de  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/shared-mime-info --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/shared-mime-info src/shared-mime-info \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dupdate-mimedb=false \
      -Dbuild-tests=false \
      -Dbuild-translations=false
ninja -C build/$QNX_ARCH/shared-mime-info
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/shared-mime-info install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- shared-mime-info: the freedesktop.org MIME type database. The database is compiled on the target with update-mime-database (it is not run at build time). Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH shared-mime-info`.

### 5. What it installs

6 files under `/usr/local`, 1 programs.

```
/usr/local/bin/update-mime-database
```

The complete list is `stage/<arch>/.manifest/shared-mime-info` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa shared-mime-info
ls dists/8.0/*/ | grep '^shared-mime-info-'
```

This writes `shared-mime-info-<ver>-r0.tar.gz` (run time) and `shared-mime-info-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add shared-mime-info
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/shared-mime-info-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/shared-mime-info-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | shared-mime-info 2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/shared-mime-info-2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | shared-mime-info 2.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/shared-mime-info-2.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add shared-mime-info=<version>`), sources in `ported/shared-mime-info/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates shared-mime-info`; port one with `scripts/port-update shared-mime-info`.

<details><summary>recipe.sh</summary>

```bash
# shared-mime-info: the freedesktop.org MIME type database. The database is compiled on the target with update-mime-database (it is not run at build time).
# Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.4
TARBALL=https://gitlab.freedesktop.org/xdg/shared-mime-info/-/archive/2.4/shared-mime-info-2.4.tar.gz
SHA256=531291d0387eb94e16e775d7e73788d06d2b2fdd8cd2ac6b6b15287593b6a2de
DEPS="glib libxml2"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dupdate-mimedb=false -Dbuild-tests=false -Dbuild-translations=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
