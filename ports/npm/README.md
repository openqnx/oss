# npm 11.11.0

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://registry.npmjs.org/npm/-/npm-11.11.0.tgz |
| SHA-256 | `cbcf4cc03148ccdb586a8bf2093c952f093fb43d5cbc97593c98b67ef8c003b0` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`nodejs`](../nodejs/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install npm
oss-get install npm-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild npm` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (13 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild nodejs
```

<details><summary>Full build chain (13)</summary>

[`ada`](../ada/) [`brotli`](../brotli/) [`c-ares`](../c-ares/) [`icu`](../icu/) [`libuv`](../libuv/) [`nghttp2`](../nghttp2/) [`nodejs`](../nodejs/) [`openssl`](../openssl/) [`simdjson`](../simdjson/) [`simdutf`](../simdutf/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://registry.npmjs.org/npm/-/npm-11.11.0.tgz
mkdir -p dist src/npm
curl -fL -o dist/$(basename $URL) $URL
echo "cbcf4cc03148ccdb586a8bf2093c952f093fb43d5cbc97593c98b67ef8c003b0  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/npm --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/npm < ports/npm/patches/0001-aports-dont-check-for-last-version.patch
```

**[`0001-aports-dont-check-for-last-version.patch`](patches/0001-aports-dont-check-for-last-version.patch)** — QNX change from QNX's aports (extra/npm/dont-check-for-last-version.patch).

> Taken from qnx-ports/aports extra/npm -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> Don't check for last version
>
> Patch based on https://sources.debian.org/src/npm/7.5.2+ds-2/debian/patches/dont-check-for-last-version.patch

Touches: `lib/cli/update-notifier.js` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/npm; B=build/$QNX_ARCH/npm; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib/node_modules/npm" "$STAGE$PREFIX/bin" && cp -a "$S/." "$STAGE$PREFIX/lib/node_modules/npm/" && \
  ln -sfn ../lib/node_modules/npm/bin/npm-cli.js "$STAGE$PREFIX/bin/npm" && ln -sfn ../lib/node_modules/npm/bin/npx-cli.js "$STAGE$PREFIX/bin/npx"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- npm, the Node.js package manager (JavaScript; runs on the target's Node.js). Installed as released into lib/node_modules/npm. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH npm`.

### 5. What it installs

1813 files under `/usr/local`, 2 programs.

```
/usr/local/bin/npm
/usr/local/bin/npx
```

The complete list is `stage/<arch>/.manifest/npm` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH npm
ls repo/8.0/*/ | grep '^npm-'
```

This writes `npm-<ver>-r0.tar.gz` (run time) and `npm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install npm
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/npm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/npm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# npm, the Node.js package manager (JavaScript; runs on the target's Node.js).
# Installed as released into lib/node_modules/npm. Version as in the QNX
# Everywhere 8.0.4r1 image.
VERSION=11.11.0
TARBALL=https://registry.npmjs.org/npm/-/npm-11.11.0.tgz
SHA256=cbcf4cc03148ccdb586a8bf2093c952f093fb43d5cbc97593c98b67ef8c003b0
DEPS="nodejs"
BUILDSYS=custom
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/lib/node_modules/npm" "$STAGE$PREFIX/bin" && cp -a "$S/." "$STAGE$PREFIX/lib/node_modules/npm/" && \
  ln -sfn ../lib/node_modules/npm/bin/npm-cli.js "$STAGE$PREFIX/bin/npm" && ln -sfn ../lib/node_modules/npm/bin/npx-cli.js "$STAGE$PREFIX/bin/npx"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
