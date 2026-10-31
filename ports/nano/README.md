# nano 8.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.nano-editor.org/dist/v8/nano-8.2.tar.xz |
| SHA-256 | `d5ad07dd862facae03051c54c6535e54c7ed7407318783fcad1ad2d7076fffeb` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`ncurses`](../ncurses/) [`zlib`](../zlib/) [`file`](../file/) |
| Run-time dependencies | [`file`](../file/) [`ncurses`](../ncurses/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add nano
apk add nano-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install nano`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nano` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses zlib file
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`file`](../file/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://www.nano-editor.org/dist/v8/nano-8.2.tar.xz
mkdir -p dist src/nano
curl -fL -o dist/$(basename $URL) $URL
echo "d5ad07dd862facae03051c54c6535e54c7ed7407318783fcad1ad2d7076fffeb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/nano --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/nano && cd build/$QNX_ARCH/nano
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/nano/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-nls \
    --enable-utf8
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): zlib Also linked at run time (found by scripts/gen-dependency-index): file

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nano`.

### 5. What it installs

53 files under `/usr/local`, 2 programs.

```
/usr/local/bin/nano
/usr/local/bin/rnano
```

The complete list is `stage/<arch>/.manifest/nano` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nano
ls dists/8.0/*/ | grep '^nano-'
```

This writes `nano-<ver>-r0.tar.gz` (run time) and `nano-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nano
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nano-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nano-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | nano 8.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/nano-8.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | nano 8.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/nano-8.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **8.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nano=<version>`), sources in `ported/nano/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nano`; port one with `scripts/port-update nano`.

<details><summary>recipe.sh</summary>

```bash
VERSION=8.2
TARBALL=https://www.nano-editor.org/dist/v8/nano-8.2.tar.xz
SHA256=d5ad07dd862facae03051c54c6535e54c7ed7407318783fcad1ad2d7076fffeb
# Also linked at run time (found by scripts/gen-dependency-index): zlib
# Also linked at run time (found by scripts/gen-dependency-index): file
DEPS="ncurses zlib file"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --enable-utf8 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
