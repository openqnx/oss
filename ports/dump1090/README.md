# dump1090 0-git-20240121

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/antirez/dump1090.git |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`rtl-sdr`](../rtl-sdr/) |
| Run-time dependencies | [`rtl-sdr`](../rtl-sdr/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add dump1090
apk add dump1090-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install dump1090`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dump1090` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild rtl-sdr
```

<details><summary>Full build chain (2)</summary>

[`libusb`](../libusb/) [`rtl-sdr`](../rtl-sdr/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/antirez/dump1090.git src/dump1090
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dump1090 < ports/dump1090/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — interactive row count fixed for qnx

> Imported from qnx-ports/dump1090 (0c3bb23eb447...port-dump1090-qnx); original authorship kept.
>
>
> ---
>  dump1090.c | 3 ++-
>  1 file changed, 2 insertions(+), 1 deletion(-)

Touches: `dump1090.c`, `Makefile` · Upstream status: QNX-specific (carried in the qnx-ports fork)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/dump1090; B=build/$QNX_ARCH/dump1090; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -DQNX=1 -I$STAGE$PREFIX/include" \
  LDLIBS="-L$STAGE$PREFIX/lib -lrtlsdr -lm -lsocket"; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/dump1090" "$STAGE$PREFIX/bin/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- antirez/dump1090 -- the original Mode S decoder for RTL-SDR dongles; this is the tree the qnx-ports port is based on. No releases; pinned commit.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dump1090`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/dump1090
```

The complete list is `stage/<arch>/.manifest/dump1090` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dump1090
ls dists/8.0/*/ | grep '^dump1090-'
```

This writes `dump1090-<ver>-r0.tar.gz` (run time) and `dump1090-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dump1090
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dump1090-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dump1090-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
No test program of its own (no run-time code to exercise, or covered by a dependent's test); `scripts/verify-on-target` installs it with apk and checks that every program and library it ships loads.

## Versions

This recipe builds **0-git-20240121**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dump1090=<version>`), sources in `ported/dump1090/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dump1090`; port one with `scripts/port-update dump1090`.

<details><summary>recipe.sh</summary>

```bash
# antirez/dump1090 -- the original Mode S decoder for RTL-SDR dongles; this is
# the tree the qnx-ports port is based on. No releases; pinned commit.
VERSION=0-git-20240121; GIT=https://github.com/antirez/dump1090.git; COMMIT=0c3bb23eb447d4ae47c7013346fa6fa97482bb1d
DEPS="rtl-sdr"
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -DQNX=1 -I$STAGE$PREFIX/include" \
  LDLIBS="-L$STAGE$PREFIX/lib -lrtlsdr -lm -lsocket"; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/dump1090" "$STAGE$PREFIX/bin/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
