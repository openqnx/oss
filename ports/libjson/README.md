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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libjson` (programs, shared libraries, run-time data) and `libjson-dev` (headers, pkg-config/CMake files, static libraries; needs `libjson`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libjson`](https://openqnx.github.io/oss/package/x86_64/libjson.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libjson.html)) · [`libjson-dev`](https://openqnx.github.io/oss/package/x86_64/libjson-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libjson-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libjson` (dependencies come along); one version: `apk add libjson=<version>` | `apk del libjson` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libjson` (dependencies first) | `oss-get remove libjson` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libjson-dev` the same way, by its own name (`apk add libjson-dev`, `oss-get install libjson-dev`, by hand with its own file and list `libjson-dev.list`); remove it before `libjson`, which it needs.

```sh
# on the QNX target, as root
apk add libjson libjson-dev
apk add qnx-test-libjson && /usr/local/bin/qnx-port-tests/test_libjson    # its on-target test
apk del libjson libjson-dev qnx-test-libjson
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libjson.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libjson.list
rm var/lib/manual/libjson.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
