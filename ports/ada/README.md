# ada 3.4.4

> Fast WHATWG-compliant URL parser library for C++

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/ada-url/ada.git (tag `v3.4.4`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ada`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ada` (programs, shared libraries, run-time data) and `ada-dev` (headers, pkg-config/CMake files, static libraries; needs `ada`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ada`](https://openqnx.github.io/oss/package/x86_64/ada.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ada.html)) · [`ada-dev`](https://openqnx.github.io/oss/package/x86_64/ada-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ada-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ada` (dependencies come along); one version: `apk add ada=<version>` | `apk del ada` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ada` (dependencies first) | `oss-get remove ada` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ada-dev` the same way, by its own name (`apk add ada-dev`, `oss-get install ada-dev`, by hand with its own file and list `ada-dev.list`); remove it before `ada`, which it needs.

```sh
# on the QNX target, as root
apk add ada ada-dev
apk add qnx-test-ada && /usr/local/bin/qnx-port-tests/test_ada    # its on-target test
apk del ada ada-dev qnx-test-ada
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ada.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ada.list
rm var/lib/manual/ada.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ada` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b v3.4.4 --recurse-submodules --shallow-submodules \
    https://github.com/ada-url/ada.git src/ada
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ada < ports/ada/patches/0001-aports-use-any-simdjson-version.patch
```

**[`0001-aports-use-any-simdjson-version.patch`](patches/0001-aports-use-any-simdjson-version.patch)** — QNX change from QNX's aports (extra/ada/use-any-simdjson-version.patch).

> Taken from qnx-ports/aports extra/ada -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.
>
>
> Don't require specific simdjson version

Touches: `CMakeLists.txt` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/ada -B build/$QNX_ARCH/ada -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DADA_TESTING=OFF \
      -DADA_TOOLS=OFF \
      -DADA_BENCHMARKS=OFF
cmake --build build/$QNX_ARCH/ada -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/ada
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ada: WHATWG URL parser (used by Node.js). Version as in the QNX Everywhere image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ada`.

### 5. What it installs

48 files under `/usr/local`: 3 shared-library entries, 40 headers.

```
/usr/local/lib/libada.so
/usr/local/lib/libada.so.3
/usr/local/lib/libada.so.3.4.4
```

The complete list is `stage/<arch>/.manifest/ada` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ada
ls dists/8.0/*/ | grep '^ada-'
```

This writes `ada-<ver>-r0.tar.gz` (run time) and `ada-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ada
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ada-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ada-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ada`](../../tests/src/) links ada the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ada`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ada
/usr/local/bin/qnx-port-tests/test_ada
```

```
== ada ==
  ok   ...
PASS ada
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ada` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ada`](../../tests/src/) -- a program linked against ada the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - parse an absolute URL
   - href is normalised (scheme, host, default port, dot segments)
   - protocol
   - hostname lower-cased
   - default port dropped
   - pathname
   - search
   - hash
   - origin
   - set_pathname percent-encodes
   - relative URL resolved against a base
   - IDNA: host converted to punycode
   - IPv4 in hex shorthand normalised
   - IPv6 compressed
   - can_parse rejects garbage
   - space in host is an error
   - file URL accepted
   - search params count
   - search param decoded
   - get_all
   - sort + serialise
   - C API: ada_parse
   - C API: port
   - C API: hostname

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ada 3.4.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/ada-3.4.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ada 3.4.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ada-3.4.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ada 3.4.4-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/ada-3.4.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ada 3.4.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ada-3.4.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.4.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ada=<version>`), sources in `ported/ada/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ada`; port one with `scripts/port-update ada`.

<details><summary>recipe.sh</summary>

```bash
# ada: WHATWG URL parser (used by Node.js). Version as in the QNX Everywhere image.
VERSION=3.4.4; GIT=https://github.com/ada-url/ada.git; REF=v3.4.4
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DADA_TESTING=OFF -DADA_TOOLS=OFF -DADA_BENCHMARKS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
