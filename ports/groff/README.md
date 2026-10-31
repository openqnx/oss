# groff 1.23.0

> GNU troff document formatting system

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz |
| SHA-256 | `6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_groff`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add groff
apk add groff-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-groff && /usr/local/bin/qnx-port-tests/test_groff    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install groff`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild groff` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libiconv
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz
mkdir -p dist src/groff
curl -fL -o dist/$(basename $URL) $URL
echo "6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/groff --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/groff; B=build/$QNX_ARCH/groff; mkdir -p $B
```

```bash
do_build()   { _host_groff && make -C "$B" -j"$(nproc)" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GNU troff (needed by man-db). Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv
- The build formats groff's own documentation and examples with groff -- the one it has just built is a QNX binary. A distribution's groff often lacks the preprocessors this needs (grn, for instance), so a groff for the build host is built from the same source first and used for that step.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH groff`.

### 5. What it installs

667 files under `/usr/local`, 39 programs.

```
/usr/local/bin/addftinfo
/usr/local/bin/afmtodit
/usr/local/bin/chem
/usr/local/bin/eqn
/usr/local/bin/eqn2graph
/usr/local/bin/gdiffmk
/usr/local/bin/glilypond
/usr/local/bin/gperl
/usr/local/bin/gpinyin
/usr/local/bin/grap2graph
/usr/local/bin/grn
/usr/local/bin/grodvi
/usr/local/bin/groff
/usr/local/bin/grog
/usr/local/bin/grolbp
/usr/local/bin/grolj4
...
```

The complete list is `stage/<arch>/.manifest/groff` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa groff
ls dists/8.0/*/ | grep '^groff-'
```

This writes `groff-<ver>-r0.tar.gz` (run time) and `groff-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add groff
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/groff-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/groff-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_groff`](../../tests/src/) links groff the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-groff`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-groff
/usr/local/bin/qnx-port-tests/test_groff
```

```
== groff ==
  ok   ...
PASS groff
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_groff` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_groff`](../../tests/src/) -- a program linked against groff the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | groff 1.23.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/groff-1.23.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | groff 1.23.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/groff-1.23.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | groff 1.23.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/groff-1.23.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | groff 1.23.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/groff-1.23.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.23.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add groff=<version>`), sources in `ported/groff/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates groff`; port one with `scripts/port-update groff`.

<details><summary>recipe.sh</summary>

```bash
# GNU troff (needed by man-db). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.23.0
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="libiconv"
TARBALL=https://ftp.gnu.org/gnu/groff/groff-1.23.0.tar.gz
SHA256=6b9757f592b7518b4902eb6af7e54570bdccba37a871fddb2d30ae3863511c13
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-x --with-doc=no --without-gs --without-uchardet )
# The build formats groff's own documentation and examples with groff -- the
# one it has just built is a QNX binary. A distribution's groff often lacks
# the preprocessors this needs (grn, for instance), so a groff for the build
# host is built from the same source first and used for that step.
_host_env() { env -u CC -u CXX -u CFLAGS -u CXXFLAGS -u CPPFLAGS -u LDFLAGS -u LIBS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR -u PKG_CONFIG_PATH "$@"; }
_host_groff() {
  local h="$B/host-groff"
  [ -x "$h/groff" ] || { mkdir -p "$h" && cd "$h" && _host_env CC=gcc CXX=g++ "$S/configure" --without-x --with-doc=no \
      --without-gs --without-uchardet && _host_env make -C "$h" -j"$(nproc)"; } || return 1
}
do_build()   { _host_groff && make -C "$B" -j"$(nproc)" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff"; }
do_install() { make -C "$B" DESTDIR="$STAGE" GROFFBIN="$B/host-groff/groff" GROFF_BIN_PATH="$B/host-groff" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
