# asio 1.30.2

> Header-only C++ library for asynchronous network and low-level I/O

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/chriskohlhoff/asio.git (tag `asio-1-30-2`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_asio`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `asio-dev` -- header-only: there is no run-time package, the -dev package is the whole component (headers, pkg-config/CMake files).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`asio-dev`](https://openqnx.github.io/oss/package/x86_64/asio-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/asio-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add asio-dev` (dependencies come along); one version: `apk add asio-dev=<version>` | `apk del asio-dev` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install asio-dev` (dependencies first) | `oss-get remove asio-dev` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-asio && /usr/local/bin/qnx-port-tests/test_asio    # its on-target test, from tests-8.0-<arch>
apk del asio-dev qnx-test-asio
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/asio-dev.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/asio-dev.list
rm var/lib/manual/asio-dev.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild asio` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b asio-1-30-2 --recurse-submodules --shallow-submodules \
    https://github.com/chriskohlhoff/asio.git src/asio
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/asio < ports/asio/patches/0001-qnx-no-sa-restart.patch
```

**[`0001-qnx-no-sa-restart.patch`](patches/0001-qnx-no-sa-restart.patch)** — Define SA_RESTART-based flag as 0 on QNX.

> QNX 8.0's <signal.h> has SA_RESTART commented out ("not supported yet"), so
> signal_set_base.hpp does not compile (restbed hits this). The same change as
> the boost port's Boost.Asio patch, for standalone Asio.

Touches: `asio/include/asio/detail/socket_types.hpp` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/asio; B=build/$QNX_ARCH/asio; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/include" && cp -a "$S/asio/include/asio" "$S/asio/include/asio.hpp" "$STAGE$PREFIX/include/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Header-only (standalone, without Boost). Upstream's autotools only installs headers, so they are copied directly.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH asio`.

### 5. What it installs

621 files under `/usr/local`, 621 headers.

The complete list is `stage/<arch>/.manifest/asio` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa asio
ls dists/8.0/*/ | grep '^asio-'
```

This writes `asio-<ver>-r0.tar.gz` (run time) and `asio-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add asio
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/asio-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/asio-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_asio`](../../tests/src/) links asio the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-asio`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-asio
/usr/local/bin/qnx-port-tests/test_asio
```

```
== asio ==
  ok   ...
PASS asio
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_asio` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_asio`](../../tests/src/) -- a program linked against asio the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - acceptor bound to 127.0.0.1, ephemeral port
   - client connected
   - three messages echoed back upper-cased
   - server accepted one connection
   - UDP datagram received, sender port right
   - steady_timer 50 ms
   - 100 handlers posted from another thread ran
   - no exception
   - make_address(192.168.1.10)
   - make_address(::1) is v6 loopback
   - make_address rejects 300.1.1.1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | asio 1.30.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/asio-1.30.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | asio 1.30.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/asio-1.30.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | asio 1.30.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/asio-1.30.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | asio 1.30.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/asio-1.30.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.30.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add asio=<version>`), sources in `ported/asio/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates asio`; port one with `scripts/port-update asio`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.30.2; GIT=https://github.com/chriskohlhoff/asio.git; REF=asio-1-30-2
# Header-only (standalone, without Boost). Upstream's autotools only installs
# headers, so they are copied directly.
BUILDSYS=custom
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/include" && cp -a "$S/asio/include/asio" "$S/asio/include/asio.hpp" "$STAGE$PREFIX/include/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
