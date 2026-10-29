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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add npm
apk add npm-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install npm`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa npm
ls dists/8.0/*/ | grep '^npm-'
```

This writes `npm-<ver>-r0.tar.gz` (run time) and `npm-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add npm
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/npm-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/npm-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **11.11.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add npm=<version>`), sources in `ported/npm/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates npm`; port one with `scripts/port-update npm`.

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
