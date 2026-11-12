# python3-can 4.6.1

> Controller Area Network interface module for Python (python-can), with a SocketCAN interface for QNX

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/74/f9/a9d99d36dd33be5badb747801c9255c3c526171a5542092eaacc73350fb8/python_can-4.6.1.tar.gz |
| SHA-256 | `290fea135d04b8504ebff33889cc6d301e2181a54099116609f940825ffe5005` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) [`python3-wrapt`](../python3-wrapt/) [`python3-packaging`](../python3-packaging/) [`python3-typing-extensions`](../python3-typing-extensions/) [`qnx-socketcan`](../qnx-socketcan/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_can`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-can`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-can`](https://openqnx.github.io/oss/package/x86_64/python3-can.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-can.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-can` (dependencies come along); one version: `apk add python3-can=<version>` | `apk del python3-can` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-can` (dependencies first) | `oss-get remove python3-can` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-can && /usr/local/bin/qnx-port-tests/test_python3_can    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-can qnx-test-python3-can
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-can.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-can.list
rm var/lib/manual/python3-can.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-can` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (19 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 python3-wrapt python3-packaging python3-typing-extensions qnx-socketcan
```

<details><summary>Full build chain (19)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-packaging`](../python3-packaging/) [`python3-typing-extensions`](../python3-typing-extensions/) [`python3-wrapt`](../python3-wrapt/) [`qnx-socketcan`](../qnx-socketcan/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/74/f9/a9d99d36dd33be5badb747801c9255c3c526171a5542092eaacc73350fb8/python_can-4.6.1.tar.gz
mkdir -p dist src/python3-can
curl -fL -o dist/$(basename $URL) $URL
echo "290fea135d04b8504ebff33889cc6d301e2181a54099116609f940825ffe5005  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-can --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/python3-can < ports/python3-can/patches/0001-qnx-socketcan-socket.patch
```

**[`0001-qnx-socketcan-socket.patch`](patches/0001-qnx-socketcan-socket.patch)** — socketcan interface on QNX: CAN sockets of qnx-socketcan, no CAN_BCM

> QNX 8.0 has no PF_CAN and Python no socket.AF_CAN. On QNX create_socket()
> returns can/interfaces/socketcan/qnx.py's CanSocket (added by the recipe from
> files/qnx.py: libqsocketcan through ctypes), find_available_interfaces() asks
> qcand, and send_periodic() uses the thread-based task (no CAN_BCM).

Touches: `can/interfaces/socketcan/socketcan.py`, `can/interfaces/socketcan/utils.py` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-can; B=build/$QNX_ARCH/python3-can; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=77" "setuptools_scm>=8" wheel; }
```

```bash
do_build() {
  cp "$RECIPE_DIR/python3-can/files/qnx.py" "$S/can/interfaces/socketcan/qnx.py" || return 1
  rm -rf "$B/wheel" && mkdir -p "$B/wheel" && \
  env -u CC -u CXX -u CFLAGS -u LDFLAGS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
    "$B/venv/bin/python" -m pip wheel --no-deps --no-build-isolation -q -w "$B/wheel" "$S" || return 1
  ls "$B"/wheel/*-none-any.whl >/dev/null 2>&1 || { echo "not a pure wheel: $(ls "$B/wheel")"; return 1; }
}
```

```bash
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- python-can: CAN bus access for Python -- can.Bus, Message, Notifier, log readers and writers (ASC, BLF, candump log, CSV, MF4, TRC, SQLite), can.logger / can.player / can.viewer. The "socketcan" interface works on QNX: QNX 8.0 has no PF_CAN and Python's socket module no AF_CAN, so on QNX the interface's socket is can/interfaces/socketcan/qnx.py (files/qnx.py), a socket object on qnx-socketcan's libqsocketcan through ctypes (the buses are qcand's: `qcand`, then can.Bus("vcan0", interface="socketcan")). Filters, CAN FD, loopback, receive_own_messages and receive timestamps work as on Linux; cyclic sending (send_periodic) uses python-can's thread-based task, as there is no CAN_BCM. Other interfaces that are pure Python work as they are: virtual, udp_multicast, socketcand, serial and slcan (python3-pyserial). Built from the source distribution into a platform-independent wheel.
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-can`.

### 5. What it installs

104 files under `/usr/local`, 5 programs.

```
/usr/local/bin/can_bridge
/usr/local/bin/can_logconvert
/usr/local/bin/can_logger
/usr/local/bin/can_player
/usr/local/bin/can_viewer
```

The complete list is `stage/<arch>/.manifest/python3-can` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-can
ls dists/8.0/*/ | grep '^python3-can-'
```

This writes `python3-can-<ver>-r0.tar.gz` (run time) and `python3-can-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-can
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-can-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-can-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_can`](../../tests/src/) links python3-can the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-can`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-can
/usr/local/bin/qnx-port-tests/test_python3_can
```

```
== python3-can ==
  ok   ...
PASS python3-can
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_can` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_can`](../../tests/src/) -- a program linked against python3-can the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-can 4.6.1-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-can-4.6.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-wrapt 2.5.2-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/aarch64le/python3-wrapt-2.5.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-can 4.6.1-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-can-4.6.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-wrapt 2.5.2-r0 | ✅ PASS | 2026-11-12 | [log](../../tests/logs/x86_64/python3-wrapt-2.5.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.6.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-can=<version>`), sources in `ported/python3-can/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-can`; port one with `scripts/port-update python3-can`.

<details><summary>recipe.sh</summary>

```bash
# python-can: CAN bus access for Python -- can.Bus, Message, Notifier,
# log readers and writers (ASC, BLF, candump log, CSV, MF4, TRC, SQLite),
# can.logger / can.player / can.viewer.
#
# The "socketcan" interface works on QNX: QNX 8.0 has no PF_CAN and Python's
# socket module no AF_CAN, so on QNX the interface's socket is
# can/interfaces/socketcan/qnx.py (files/qnx.py), a socket object on
# qnx-socketcan's libqsocketcan through ctypes (the buses are qcand's:
# `qcand`, then can.Bus("vcan0", interface="socketcan")). Filters, CAN FD,
# loopback, receive_own_messages and receive timestamps work as on Linux;
# cyclic sending (send_periodic) uses python-can's thread-based task, as
# there is no CAN_BCM. Other interfaces that are pure Python work as they
# are: virtual, udp_multicast, socketcand, serial and slcan (python3-pyserial).
# Built from the source distribution into a platform-independent wheel.
VERSION=4.6.1
TARBALL=https://files.pythonhosted.org/packages/74/f9/a9d99d36dd33be5badb747801c9255c3c526171a5542092eaacc73350fb8/python_can-4.6.1.tar.gz
SHA256=290fea135d04b8504ebff33889cc6d301e2181a54099116609f940825ffe5005
DEPS="python3 python3-wrapt python3-packaging python3-typing-extensions qnx-socketcan"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-wrapt python3-packaging python3-typing-extensions qnx-socketcan"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77" "setuptools_scm>=8" wheel; }
do_build() {
  cp "$RECIPE_DIR/python3-can/files/qnx.py" "$S/can/interfaces/socketcan/qnx.py" || return 1
  rm -rf "$B/wheel" && mkdir -p "$B/wheel" && \
  env -u CC -u CXX -u CFLAGS -u LDFLAGS -u PKG_CONFIG_LIBDIR -u PKG_CONFIG_SYSROOT_DIR \
    "$B/venv/bin/python" -m pip wheel --no-deps --no-build-isolation -q -w "$B/wheel" "$S" || return 1
  ls "$B"/wheel/*-none-any.whl >/dev/null 2>&1 || { echo "not a pure wheel: $(ls "$B/wheel")"; return 1; }
}
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
