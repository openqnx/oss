# tree-sitter-bash 0.25.0

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/tree-sitter/tree-sitter-bash.git (tag `v0.25.0`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ts_grammars`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add tree-sitter-bash
apk add tree-sitter-bash-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-ts-grammars && /usr/local/bin/qnx-port-tests/test_ts_grammars    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install tree-sitter-bash`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tree-sitter-bash` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v0.25.0 --recurse-submodules --shallow-submodules \
    https://github.com/tree-sitter/tree-sitter-bash.git src/tree-sitter-bash
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tree-sitter-bash; B=build/$QNX_ARCH/tree-sitter-bash; mkdir -p $B
```

```bash
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c "$d"/scanner.cc; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/bash.so" "${objs[@]}"
}
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/bash.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tree-sitter grammar: bash. Installs lib/tree-sitter/bash.so (the parser Neovim and the tree-sitter library load). Version as in QNX's repository.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tree-sitter-bash`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/tree-sitter-bash` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tree-sitter-bash
ls dists/8.0/*/ | grep '^tree-sitter-bash-'
```

This writes `tree-sitter-bash-<ver>-r0.tar.gz` (run time) and `tree-sitter-bash-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tree-sitter-bash
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tree-sitter-bash-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tree-sitter-bash-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ts_grammars`](../../tests/src/) links tree-sitter-bash the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ts-grammars`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ts-grammars
/usr/local/bin/qnx-port-tests/test_ts_grammars
```

```
== tree-sitter-bash ==
  ok   ...
PASS tree-sitter-bash
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ts_grammars` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ts_grammars`](../../tests/src/) -- a program linked against tree-sitter-bash the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tree-sitter-bash 0.25.0-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tree-sitter-cmake 0.7.2-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tree-sitter-make 1.1.1-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tree-sitter-python 0.23.6-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tree-sitter-bash 0.25.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tree-sitter-cmake 0.7.2-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tree-sitter-make 1.1.1-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tree-sitter-python 0.23.6-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **0.25.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tree-sitter-bash=<version>`), sources in `ported/tree-sitter-bash/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tree-sitter-bash`; port one with `scripts/port-update tree-sitter-bash`.

<details><summary>recipe.sh</summary>

```bash
# tree-sitter grammar: bash. Installs lib/tree-sitter/bash.so (the parser
# Neovim and the tree-sitter library load). Version as in QNX's repository.
VERSION=0.25.0; GIT=https://github.com/tree-sitter/tree-sitter-bash.git; REF=v0.25.0
BUILDSYS=custom
do_build() {
  local d="$S/src" f objs=()
  for f in "$d"/parser.c "$d"/scanner.c "$d"/scanner.cc; do [ -f "$f" ] && objs+=("$f"); done
  qcc -V$QCC_TARGET -O2 -fPIC -shared -D_QNX_SOURCE -I"$d" -o "$B/bash.so" "${objs[@]}"
}
do_install() { install -d "$STAGE$PREFIX/lib/tree-sitter" && install -m755 "$B/bash.so" "$STAGE$PREFIX/lib/tree-sitter/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
