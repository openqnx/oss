# libjson 0.8-git-20260512

> Small streaming JSON parser and printer in C

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/vincenthz/libjson.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libjson`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libjson
apk add libjson-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libjson && /usr/local/bin/qnx-port-tests/test_libjson    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libjson`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libjson` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/vincenthz/libjson.git src/libjson
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libjson; B=build/$QNX_ARCH/libjson; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -fPIC -D_QNX_SOURCE" PREFIX="$PREFIX"; }
```

```bash
do_install() { make -C "$B" PREFIX="$PREFIX" DESTDIR="$STAGE" install-lib; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- vincenthz/libjson: a small streaming JSON parser/printer in C. No releases; pinned to the last commit.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libjson`.

### 5. What it installs

7 files under `/usr/local`: 4 shared-library entries, 1 headers.

```
/usr/local/lib/libjson.so
/usr/local/lib/libjson.so.1
/usr/local/lib/libjson.so.1.0
/usr/local/lib/libjson.so.1.0.0
```

The complete list is `stage/<arch>/.manifest/libjson` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libjson
ls dists/8.0/*/ | grep '^libjson-'
```

This writes `libjson-<ver>-r0.tar.gz` (run time) and `libjson-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libjson
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libjson-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libjson-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libjson`](../../tests/src/) links libjson the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libjson`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libjson
/usr/local/bin/qnx-port-tests/test_libjson
```

```
== libjson ==
  ok   ...
PASS libjson
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libjson` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libjson`](../../tests/src/) -- a program linked against libjson the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - document parses
   - event sequence: object, keys, string, int, floats, true, null
   - key and value text
   - escapes decoded (\\\" and \\u00e9 -> UTF-8)
   - parse into the raw printer
   - parse -> print_raw reproduces the compact text
   - byte-at-a-time feeding
   - C comments accepted when enabled
   - comments rejected by default
   - missing colon is an error
   - nesting limit enforced
   - truncated input is not done

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libjson 0.8_git20260512-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libjson-0.8_git20260512-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libjson 0.8_git20260512-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libjson-0.8_git20260512-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libjson 0.8_git20260512-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libjson-0.8_git20260512-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libjson 0.8_git20260512-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libjson-0.8_git20260512-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.8-git-20260512**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libjson=<version>`), sources in `ported/libjson/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libjson`; port one with `scripts/port-update libjson`.

<details><summary>recipe.sh</summary>

```bash
# vincenthz/libjson: a small streaming JSON parser/printer in C. No releases;
# pinned to the last commit.
VERSION=0.8-git-20260512; GIT=https://github.com/vincenthz/libjson.git; COMMIT=da08310c7a7b7919205425dfa9469d89accb7aee
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -fPIC -D_QNX_SOURCE" PREFIX="$PREFIX"; }
do_install() { make -C "$B" PREFIX="$PREFIX" DESTDIR="$STAGE" install-lib; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
