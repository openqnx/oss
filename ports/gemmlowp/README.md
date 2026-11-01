# gemmlowp 0-git-20231103

> Small self-contained low-precision (8-bit) GEMM library

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/google/gemmlowp.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gemmlowp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `gemmlowp-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gemmlowp-dev`](https://openqnx.github.io/oss/package/x86_64/gemmlowp-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gemmlowp-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gemmlowp-dev` (dependencies come along); one version: `apk add gemmlowp-dev=<version>` | `apk del gemmlowp-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gemmlowp-dev` (dependencies first) | `oss-get remove gemmlowp-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-gemmlowp && /usr/local/bin/qnx-port-tests/test_gemmlowp    # its on-target test, from tests-8.0-<arch>
apk del gemmlowp-dev qnx-test-gemmlowp
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gemmlowp-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gemmlowp-dev.list
rm var/lib/manual/gemmlowp-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gemmlowp` performs exactly the same steps (2–5) in one command.

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
    https://github.com/google/gemmlowp.git src/gemmlowp
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/gemmlowp; B=build/$QNX_ARCH/gemmlowp; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local d
  for d in fixedpoint internal meta profiling public eight_bit_int_gemm; do
    install -d "$STAGE$PREFIX/include/gemmlowp/$d" || return 1
    find "$S/$d" -maxdepth 1 -name '*.h' -exec install -m644 {} "$STAGE$PREFIX/include/gemmlowp/$d/" \; || return 1
  done
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Google's low-precision (8-bit) matrix multiplication library. No releases; pinned to the last commit. Used header-only (as TensorFlow does): the headers go to include/gemmlowp/. Upstream's contrib/CMakeLists.txt builds only its tests and benchmarks and installs nothing.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gemmlowp`.

### 5. What it installs

66 files under `/usr/local`, 66 headers.

The complete list is `stage/<arch>/.manifest/gemmlowp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gemmlowp
ls dists/8.0/*/ | grep '^gemmlowp-'
```

This writes `gemmlowp-<ver>-r0.tar.gz` (run time) and `gemmlowp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gemmlowp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gemmlowp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gemmlowp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gemmlowp`](../../tests/src/) links gemmlowp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gemmlowp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gemmlowp
/usr/local/bin/qnx-port-tests/test_gemmlowp
```

```
== gemmlowp ==
  ok   ...
PASS gemmlowp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gemmlowp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gemmlowp`](../../tests/src/) -- a program linked against gemmlowp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - uint8 pipeline (quantize down by scale + saturating cast) = reference
   - fixed-point exp_on_negative_values(-1) = e^-1
   - fixed-point logistic(0) = 0.5, tanh(0.5)
   - Q0.31 multiply 0.25 * 0.5 = 0.125

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> gemmlowp`.

## Versions

This recipe builds **0-git-20231103**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gemmlowp=<version>`), sources in `ported/gemmlowp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gemmlowp`; port one with `scripts/port-update gemmlowp`.

<details><summary>recipe.sh</summary>

```bash
# Google's low-precision (8-bit) matrix multiplication library. No releases;
# pinned to the last commit. Used header-only (as TensorFlow does): the
# headers go to include/gemmlowp/. Upstream's contrib/CMakeLists.txt builds
# only its tests and benchmarks and installs nothing.
VERSION=0-git-20231103; GIT=https://github.com/google/gemmlowp.git; COMMIT=16e8662c34917be0065110bfcd9cc27d30f52fdf
BUILDSYS=custom
do_build() { :; }
do_install() {
  local d
  for d in fixedpoint internal meta profiling public eight_bit_int_gemm; do
    install -d "$STAGE$PREFIX/include/gemmlowp/$d" || return 1
    find "$S/$d" -maxdepth 1 -name '*.h' -exec install -m644 {} "$STAGE$PREFIX/include/gemmlowp/$d/" \; || return 1
  done
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
