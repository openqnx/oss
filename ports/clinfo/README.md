# clinfo 3.1.26.09.26

> Prints all properties of the available OpenCL platforms and devices (text, raw or JSON)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/Oblomov/clinfo.git (tag `3.1.26.09.26`) |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`opencl-headers`](../opencl-headers/) [`opencl-icd-loader`](../opencl-icd-loader/) |
| Run-time dependencies | [`opencl-icd-loader`](../opencl-icd-loader/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_clinfo`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `clinfo` (programs, shared libraries, run-time data) and `clinfo-dev` (headers, pkg-config/CMake files, static libraries; needs `clinfo`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`clinfo`](https://openqnx.github.io/oss/package/x86_64/clinfo.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/clinfo.html)) · [`clinfo-dev`](https://openqnx.github.io/oss/package/x86_64/clinfo-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/clinfo-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add clinfo` (dependencies come along); one version: `apk add clinfo=<version>` | `apk del clinfo` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install clinfo` (dependencies first) | `oss-get remove clinfo` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`clinfo-dev` the same way, by its own name (`apk add clinfo-dev`, `oss-get install clinfo-dev`, by hand with its own file and list `clinfo-dev.list`); remove it before `clinfo`, which it needs.

```sh
# on the QNX target, as root
apk add clinfo clinfo-dev
apk add qnx-test-clinfo && /usr/local/bin/qnx-port-tests/test_clinfo    # its on-target test, from tests-8.0-<arch>
apk del clinfo clinfo-dev qnx-test-clinfo
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/clinfo.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/clinfo.list
rm var/lib/manual/clinfo.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild clinfo` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild opencl-headers opencl-icd-loader
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.1.26.09.26 --recurse-submodules --shallow-submodules \
    https://github.com/Oblomov/clinfo.git src/clinfo
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/clinfo; B=build/$QNX_ARCH/clinfo; mkdir -p $B
```

```bash
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -std=c99 -Wall -Wextra -I"$STAGE$PREFIX/include" \
    -o "$B/clinfo" "$S/src/clinfo.c" -L"$STAGE$PREFIX/lib" -lOpenCL
}
```

```bash
do_install() {
  install -Dm755 "$B/clinfo" "$STAGE$PREFIX/bin/clinfo"
  install -Dm644 "$S/man1/clinfo.1" "$STAGE$PREFIX/share/man/man1/clinfo.1"
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/clinfo/LICENSE"
  install -m644 "$S/legalcode.txt" "$STAGE$PREFIX/share/licenses/clinfo/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- clinfo: prints every property of the OpenCL platforms and devices the ICD loader finds -- names, versions, extensions, device limits, the loader's own profile -- as text (default, --human), raw values (--raw) or JSON (--json); -l lists platforms and devices as a tree. The first thing to run when an OpenCL program finds no device. Without an installed OpenCL implementation it prints "Number of platforms 0" and exits 0. The source is one portable C99 file and needed no change for QNX. Upstream's Makefile compiles into the source directory (one tree for both architectures here) and links -ldl, which QNX does not have (dlopen() is in libc), so the file is compiled directly: against opencl-headers and opencl-icd-loader's libOpenCL.so.1. The manual page goes to share/man.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH clinfo`.

### 5. What it installs

4 files under `/usr/local`, 1 programs.

```
/usr/local/bin/clinfo
```

The complete list is `stage/<arch>/.manifest/clinfo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa clinfo
ls dists/8.0/*/ | grep '^clinfo-'
```

This writes `clinfo-<ver>-r0.tar.gz` (run time) and `clinfo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add clinfo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/clinfo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/clinfo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_clinfo`](../../tests/src/) links clinfo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-clinfo`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-clinfo
/usr/local/bin/qnx-port-tests/test_clinfo
```

```
== clinfo ==
  ok   ...
PASS clinfo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_clinfo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_clinfo`](../../tests/src/) -- a program linked against clinfo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | clinfo 3.1.26.09.26-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/aarch64le/clinfo-3.1.26.09.26-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | clinfo 3.1.26.09.26-r0 | ✅ PASS | 2026-11-13 | [log](../../tests/logs/x86_64/clinfo-3.1.26.09.26-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.1.26.09.26**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add clinfo=<version>`), sources in `ported/clinfo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates clinfo`; port one with `scripts/port-update clinfo`.

<details><summary>recipe.sh</summary>

```bash
# clinfo: prints every property of the OpenCL platforms and devices the ICD
# loader finds -- names, versions, extensions, device limits, the loader's
# own profile -- as text (default, --human), raw values (--raw) or JSON
# (--json); -l lists platforms and devices as a tree. The first thing to run
# when an OpenCL program finds no device. Without an installed OpenCL
# implementation it prints "Number of platforms 0" and exits 0.
#
# The source is one portable C99 file and needed no change for QNX.
# Upstream's Makefile compiles into the source directory (one tree for both
# architectures here) and links -ldl, which QNX does not have (dlopen() is in
# libc), so the file is compiled directly: against opencl-headers and
# opencl-icd-loader's libOpenCL.so.1. The manual page goes to share/man.
VERSION=3.1.26.09.26; GIT=https://github.com/Oblomov/clinfo.git; REF=3.1.26.09.26
DEPS="opencl-headers opencl-icd-loader"
BUILDSYS=custom
do_build() {
  "$QNX_HOST/usr/bin/qcc" -V"$QCC_TARGET" -O2 -std=c99 -Wall -Wextra -I"$STAGE$PREFIX/include" \
    -o "$B/clinfo" "$S/src/clinfo.c" -L"$STAGE$PREFIX/lib" -lOpenCL
}
do_install() {
  install -Dm755 "$B/clinfo" "$STAGE$PREFIX/bin/clinfo"
  install -Dm644 "$S/man1/clinfo.1" "$STAGE$PREFIX/share/man/man1/clinfo.1"
  install -Dm644 "$S/LICENSE" "$STAGE$PREFIX/share/licenses/clinfo/LICENSE"
  install -m644 "$S/legalcode.txt" "$STAGE$PREFIX/share/licenses/clinfo/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
