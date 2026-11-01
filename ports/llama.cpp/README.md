# llama.cpp 0_b9006

> LLM inference in C/C++

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/ggml-org/llama.cpp.git (tag `b9006`) |
| Build system | CMake |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | [`openssl`](../openssl/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_llama_cpp`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `llama.cpp`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`llama.cpp`](https://openqnx.github.io/oss/package/x86_64/llama.cpp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/llama.cpp.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add llama.cpp` (dependencies come along); one version: `apk add llama.cpp=<version>` | `apk del llama.cpp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install llama.cpp` (dependencies first) | `oss-get remove llama.cpp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-llama-cpp && /usr/local/bin/qnx-port-tests/test_llama_cpp    # its on-target test, from tests-8.0-<arch>
apk del llama.cpp qnx-test-llama-cpp
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/llama.cpp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/llama.cpp.list
rm var/lib/manual/llama.cpp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild llama.cpp` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b b9006 --recurse-submodules --shallow-submodules \
    https://github.com/ggml-org/llama.cpp.git src/llama.cpp
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/llama.cpp < ports/llama.cpp/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/llama.cpp < ports/llama.cpp/patches/0002-qnx-limits-cache-dir.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — llama.cpp on QNX, from QNX's port (github.com/qnx-ports/llama.cpp, branch

> qnx-8.0, commit 84e5a4e2): the Vulkan backend's shader-compile counter is
> released by a guard, so a pipeline creation that throws (the virtio-gpu
> Venus driver on QNX QEMU set-ups returns VK_ERROR_INITIALIZATION_FAILED)
> no longer leaves ggml_vk_load_shaders waiting for ever. Not built here:
> the SDP has no Vulkan. The rest of QNX's branch is upstream b9006.

Touches: `ggml/src/ggml-vulkan/ggml-vulkan.cpp` · Upstream status: Inappropriate [QNX's port, kept as QNX published it]

**[`0002-qnx-limits-cache-dir.patch`](patches/0002-qnx-limits-cache-dir.patch)** — QNX: PATH_MAX from <limits.h>, and the XDG cache directory.

> QNX has no <sys/syslimits.h>, the header arg.cpp and download.cpp take
> PATH_MAX from on systems other than Linux, Windows and AIX; QNX's PATH_MAX
> is in <limits.h>. And fs_get_cache_directory() knew no cache directory for
> QNX and stopped the build with "#error Unknown architecture": QNX takes the
> XDG/HOME rule of Linux and the BSDs ($XDG_CACHE_HOME, else
> $HOME/.cache/llama.cpp), as the BSDs do, without the getpwuid() fallback.

Touches: `common/arg.cpp`, `common/common.cpp`, `common/download.cpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/llama.cpp; B=build/$QNX_ARCH/llama.cpp; mkdir -p $B
```

```bash
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  for p in "$STAGE$OPT"/bin/llama-*; do [ -e "$p" ] && ln -sf "../opt/llama.cpp/bin/${p##*/}" "$STAGE$PREFIX/bin/${p##*/}"; done; return 0
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- llama.cpp: LLM inference in C/C++ on ggml -- the llama library, llama-cli, llama-server, llama-quantize and the other tools, CPU inference -- with QNX's change (patches/0001, from github.com/qnx-ports/llama.cpp, branch qnx-8.0; release b9006 is the commit QNX's port is based on).
- It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and whisper.cpp each do, in versions that are not interchangeable: each gets a prefix of its own, /usr/local/opt/llama.cpp, its commands linked into bin/. The run path is that prefix's lib/, absolute: QNX's loader does not expand $ORIGIN (docs/porting-hazards.md). No libcurl (models are given as files, not downloaded), no OpenMP (QNX's GCC has no OpenMP runtime: ggml's own thread pool), no Vulkan (not in the SDP).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH llama.cpp`.

### 5. What it installs

83 files under `/usr/local`, 18 programs.

```
/usr/local/bin/llama-batched-bench
/usr/local/bin/llama-bench
/usr/local/bin/llama-cli
/usr/local/bin/llama-completion
/usr/local/bin/llama-cvector-generator
/usr/local/bin/llama-debug-template-parser
/usr/local/bin/llama-export-lora
/usr/local/bin/llama-fit-params
/usr/local/bin/llama-gguf-split
/usr/local/bin/llama-imatrix
/usr/local/bin/llama-mtmd-cli
/usr/local/bin/llama-perplexity
/usr/local/bin/llama-quantize
/usr/local/bin/llama-results
/usr/local/bin/llama-server
/usr/local/bin/llama-template-analysis
...
```

The complete list is `stage/<arch>/.manifest/llama.cpp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa llama.cpp
ls dists/8.0/*/ | grep '^llama.cpp-'
```

This writes `llama.cpp-<ver>-r0.tar.gz` (run time) and `llama.cpp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add llama.cpp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/llama.cpp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/llama.cpp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_llama_cpp`](../../tests/src/) links llama.cpp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-llama-cpp`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-llama-cpp
/usr/local/bin/qnx-port-tests/test_llama_cpp
```

```
== llama.cpp ==
  ok   ...
PASS llama.cpp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_llama_cpp` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_llama_cpp`](../../tests/src/) -- a program linked against llama.cpp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - tiny GGUF model written with the gguf API
   - model loaded (llama architecture, 2 blocks)
   - hyperparameters read back (32 tokens, 32 wide, 2 layers)
   - \"abc\" tokenized as \"▁\" a b c (ids 3 4 5 6)
   - detokenized back to \"abc\"
   - with special tokens: <s> first
   - context created (KV cache for 64 tokens)
   - prompt decoded
   - next-token logits: d = sqrt(32) = 5.657, every other token 0
   - greedy decoding: \"defghijklmnopqrstuvwxyz.\"
   - generation stops at end of text (</s>)

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | llama.cpp 0_git-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/llama.cpp-0_git-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | llama.cpp 0_git-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/llama.cpp-0_git-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0_b9006**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add llama.cpp=<version>`), sources in `ported/llama.cpp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates llama.cpp`; port one with `scripts/port-update llama.cpp`.

<details><summary>recipe.sh</summary>

```bash
# llama.cpp: LLM inference in C/C++ on ggml -- the llama library, llama-cli,
# llama-server, llama-quantize and the other tools, CPU inference -- with
# QNX's change (patches/0001, from github.com/qnx-ports/llama.cpp, branch
# qnx-8.0; release b9006 is the commit QNX's port is based on).
VERSION=0_b9006; GIT=https://github.com/ggml-org/llama.cpp.git; REF=b9006
DEPS=""
BUILDSYS=cmake
# It carries its own ggml (libggml*.so, ggml.h), as stable-diffusion.cpp and
# whisper.cpp each do, in versions that are not interchangeable: each gets a
# prefix of its own, /usr/local/opt/llama.cpp, its commands linked into bin/.
# The run path is that prefix's lib/, absolute: QNX's loader does not expand
# $ORIGIN (docs/porting-hazards.md).
# No libcurl (models are given as files, not downloaded), no OpenMP (QNX's GCC
# has no OpenMP runtime: ggml's own thread pool), no Vulkan (not in the SDP).
OPT="$PREFIX/opt/llama.cpp"
CMAKE_ARGS_EXTRA=( -DCMAKE_INSTALL_PREFIX="$OPT" -DCMAKE_INSTALL_RPATH="$OPT/lib" -DCMAKE_BUILD_WITH_INSTALL_RPATH=ON -DBUILD_SHARED_LIBS=ON
                   -DLLAMA_BUILD_TESTS=OFF -DLLAMA_BUILD_EXAMPLES=OFF -DLLAMA_BUILD_TOOLS=ON -DLLAMA_BUILD_SERVER=ON -DLLAMA_CURL=OFF
                   -DGGML_NATIVE=OFF -DGGML_OPENMP=OFF -DGGML_VULKAN=OFF -DGGML_BACKEND_DL=OFF -DLLAMA_BUILD_NUMBER=9006 )
do_install() {
  DESTDIR="$STAGE" cmake --install "$B" || return 1
  local p; mkdir -p "$STAGE$PREFIX/bin"
  for p in "$STAGE$OPT"/bin/llama-*; do [ -e "$p" ] && ln -sf "../opt/llama.cpp/bin/${p##*/}" "$STAGE$PREFIX/bin/${p##*/}"; done; return 0
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
