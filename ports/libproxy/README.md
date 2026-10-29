# libproxy 0.5.9

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/libproxy/libproxy.git (tag `0.5.9`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`curl`](../curl/) |
| Run-time dependencies | [`curl`](../curl/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libproxy
apk add libproxy-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libproxy`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libproxy` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild glib curl
```

<details><summary>Full build chain (15)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.5.9 --recurse-submodules --shallow-submodules \
    https://github.com/libproxy/libproxy.git src/libproxy
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/libproxy src/libproxy \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtests=false \
      -Ddocs=false \
      -Dintrospection=false \
      -Dvapi=false \
      -Dconfig-gnome=false \
      -Dconfig-kde=false \
      -Dconfig-osx=false \
      -Dconfig-windows=false \
      -Dpacrunner-duktape=false \
      -Dcurl=true
ninja -C build/$QNX_ARCH/libproxy
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/libproxy install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libproxy`.

### 5. What it installs

8 files under `/usr/local`: 3 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libproxy.so
/usr/local/lib/libproxy.so.0.5.9
/usr/local/lib/libproxy.so.1
/usr/local/bin/proxy
```

The complete list is `stage/<arch>/.manifest/libproxy` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libproxy
ls dists/8.0/*/ | grep '^libproxy-'
```

This writes `libproxy-<ver>-r0.tar.gz` (run time) and `libproxy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libproxy
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libproxy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libproxy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **0.5.9**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libproxy=<version>`), sources in `ported/libproxy/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libproxy`; port one with `scripts/port-update libproxy`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.5.9; GIT=https://github.com/libproxy/libproxy.git; REF=0.5.9
DEPS="glib curl"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtests=false -Ddocs=false -Dintrospection=false -Dvapi=false
                   -Dconfig-gnome=false -Dconfig-kde=false -Dconfig-osx=false
                   -Dconfig-windows=false -Dpacrunner-duktape=false
                   -Dcurl=true )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
