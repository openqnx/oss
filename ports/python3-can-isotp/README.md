# python3-can-isotp 2.0.7

> ISO-TP (ISO 15765-2) transport protocol for Python

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/e4/1e/f1b163783c90ccbad0227de267d899d7283b3174b89f1d66bb7c2a509808/can_isotp-2.0.7-py3-none-any.whl |
| SHA-256 | `28d155dec1c640bf32771fb4e271323d777158ab130372724a8b7b82aca66a7c` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) [`python3-can`](../python3-can/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_canlibs`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-can-isotp`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-can-isotp`](https://openqnx.github.io/oss/package/x86_64/python3-can-isotp.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-can-isotp.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-can-isotp` (dependencies come along); one version: `apk add python3-can-isotp=<version>` | `apk del python3-can-isotp` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-can-isotp` (dependencies first) | `oss-get remove python3-can-isotp` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-canlibs && /usr/local/bin/qnx-port-tests/test_python3_canlibs    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-can-isotp qnx-test-python3-canlibs
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-can-isotp.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-can-isotp.list
rm var/lib/manual/python3-can-isotp.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-can-isotp` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 python3-can
```

<details><summary>Full build chain (20)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-can`](../python3-can/) [`python3-packaging`](../python3-packaging/) [`python3-typing-extensions`](../python3-typing-extensions/) [`python3-wrapt`](../python3-wrapt/) [`qnx-socketcan`](../qnx-socketcan/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/e4/1e/f1b163783c90ccbad0227de267d899d7283b3174b89f1d66bb7c2a509808/can_isotp-2.0.7-py3-none-any.whl
mkdir -p dist src/python3-can-isotp
curl -fL -o dist/$(basename $URL) $URL
echo "28d155dec1c640bf32771fb4e271323d777158ab130372724a8b7b82aca66a7c  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-can-isotp --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-can-isotp; B=build/$QNX_ARCH/python3-can-isotp; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- can-isotp: ISO 15765-2 (ISO-TP) transport for Python. Its TransportLayer is the protocol in Python and runs on any python-can bus (isotp.CanStack / NotifierBasedCanStack: on QNX the socketcan interface on qnx-socketcan's buses). isotp.socket, the Linux kernel's CAN_ISOTP socket, is not available on QNX (qnx-socketcan has CAN_RAW only). Pure Python: the published wheel is installed as released (scripts/lib/pywheel.sh).
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-can-isotp`.

### 5. What it installs

14 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-can-isotp` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-can-isotp
ls dists/8.0/*/ | grep '^python3-can-isotp-'
```

This writes `python3-can-isotp-<ver>-r0.tar.gz` (run time) and `python3-can-isotp-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-can-isotp
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-can-isotp-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-can-isotp-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_canlibs`](../../tests/src/) links python3-can-isotp the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-canlibs`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-canlibs
/usr/local/bin/qnx-port-tests/test_python3_canlibs
```

```
== python3-can-isotp ==
  ok   ...
PASS python3-can-isotp
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_canlibs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_canlibs`](../../tests/src/) -- a program linked against python3-can-isotp the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-argparse-addons 0.12.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-argparse-addons-0.12.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-bitstruct 8.24.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-bitstruct-8.24.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-can-isotp 2.0.7-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-can-isotp-2.0.7-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-can-j1939 2.0.12-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-can-j1939-2.0.12-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-canmatrix 1.3.3-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-canmatrix-1.3.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-cantools 45.1.2-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-cantools-45.1.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-crccheck 1.3.1-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-crccheck-1.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pyserial 3.5-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-pyserial-3.5-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-textparser 0.26.2-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-textparser-0.26.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-udsoncan 1.26.1-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-udsoncan-1.26.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-argparse-addons 0.12.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-argparse-addons-0.12.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-bitstruct 8.24.0-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-bitstruct-8.24.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-can-isotp 2.0.7-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-can-isotp-2.0.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-can-j1939 2.0.12-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-can-j1939-2.0.12-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-canmatrix 1.3.3-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-canmatrix-1.3.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-cantools 45.1.2-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-cantools-45.1.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-crccheck 1.3.1-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-crccheck-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pyserial 3.5-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-pyserial-3.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-textparser 0.26.2-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-textparser-0.26.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-udsoncan 1.26.1-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-udsoncan-1.26.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.0.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-can-isotp=<version>`), sources in `ported/python3-can-isotp/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-can-isotp`; port one with `scripts/port-update python3-can-isotp`.

<details><summary>recipe.sh</summary>

```bash
# can-isotp: ISO 15765-2 (ISO-TP) transport for Python. Its TransportLayer
# is the protocol in Python and runs on any python-can bus
# (isotp.CanStack / NotifierBasedCanStack: on QNX the socketcan interface on
# qnx-socketcan's buses). isotp.socket, the Linux kernel's CAN_ISOTP socket,
# is not available on QNX (qnx-socketcan has CAN_RAW only). Pure Python: the
# published wheel is installed as released (scripts/lib/pywheel.sh).
VERSION=2.0.7
TARBALL=https://files.pythonhosted.org/packages/e4/1e/f1b163783c90ccbad0227de267d899d7283b3174b89f1d66bb7c2a509808/can_isotp-2.0.7-py3-none-any.whl
SHA256=28d155dec1c640bf32771fb4e271323d777158ab130372724a8b7b82aca66a7c
DEPS="python3 python3-can"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-can"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
