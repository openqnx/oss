# meson 1.10.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/32/4f/c398c6f06ece1c6c246e008d5dac3824c98f54d3eb3d8014f4910afd6d48/meson-1.10.0-py3-none-any.whl |
| SHA-256 | `4b27aafce281e652dcb437b28007457411245d975c48b5db3a797d3e93ae1585` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add meson
apk add meson-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install meson`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild meson` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (15 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3
```

<details><summary>Full build chain (15)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/32/4f/c398c6f06ece1c6c246e008d5dac3824c98f54d3eb3d8014f4910afd6d48/meson-1.10.0-py3-none-any.whl
mkdir -p dist src/meson
curl -fL -o dist/$(basename $URL) $URL
echo "4b27aafce281e652dcb437b28007457411245d975c48b5db3a797d3e93ae1585  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/meson --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/meson; B=build/$QNX_ARCH/meson; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure && install -d "$STAGE$PREFIX/bin" && \
  printf '#!/usr/local/bin/python3\nimport sys\nfrom mesonbuild.mesonmain import main\nsys.exit(main())\n' > "$STAGE$PREFIX/bin/meson" && \
  chmod 755 "$STAGE$PREFIX/bin/meson"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Meson, the build system, for the target (runs on the target's Python). Pure Python: the published wheel is installed as released. Version as in the QNX Everywhere 8.0.4r1 image.
- The wheel's console script entry point is written as a small launcher.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH meson`.

### 5. What it installs

255 files under `/usr/local`, 1 programs.

```
/usr/local/bin/meson
```

The complete list is `stage/<arch>/.manifest/meson` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa meson
ls dists/8.0/*/ | grep '^meson-'
```

This writes `meson-<ver>-r0.tar.gz` (run time) and `meson-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add meson
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/meson-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/meson-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **1.10.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add meson=<version>`), sources in `ported/meson/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates meson`; port one with `scripts/port-update meson`.

<details><summary>recipe.sh</summary>

```bash
# Meson, the build system, for the target (runs on the target's Python).
# Pure Python: the published wheel is installed as released. Version as in the
# QNX Everywhere 8.0.4r1 image.
VERSION=1.10.0
TARBALL=https://files.pythonhosted.org/packages/32/4f/c398c6f06ece1c6c246e008d5dac3824c98f54d3eb3d8014f4910afd6d48/meson-1.10.0-py3-none-any.whl
SHA256=4b27aafce281e652dcb437b28007457411245d975c48b5db3a797d3e93ae1585
DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
# The wheel's console script entry point is written as a small launcher.
do_install() { qnx_py_install_pure && install -d "$STAGE$PREFIX/bin" && \
  printf '#!/usr/local/bin/python3\nimport sys\nfrom mesonbuild.mesonmain import main\nsys.exit(main())\n' > "$STAGE$PREFIX/bin/meson" && \
  chmod 755 "$STAGE$PREFIX/bin/meson"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
