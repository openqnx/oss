# python3-zstandard 0.25.0

> Zstandard compression bindings for Python (C backend on the system zstd)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/fd/aa/3e0508d5a5dd96529cdc5a97011299056e14c6505b678fd58938792794b1/zstandard-0.25.0.tar.gz |
| SHA-256 | `7713e1179d162cf5c7906da876ec2ccb9c3a9dcbdffef0cc7f70c3667a205f0b` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) [`zstd`](../zstd/) |
| Run-time dependencies | [`zstd`](../zstd/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_msgpack_zstandard`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-zstandard`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-zstandard`](https://openqnx.github.io/oss/package/x86_64/python3-zstandard.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-zstandard.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-zstandard` (dependencies come along); one version: `apk add python3-zstandard=<version>` | `apk del python3-zstandard` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-zstandard` (dependencies first) | `oss-get remove python3-zstandard` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-msgpack-zstandard && /usr/local/bin/qnx-port-tests/test_python3_msgpack_zstandard    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-zstandard qnx-test-python3-msgpack-zstandard
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-zstandard.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-zstandard.list
rm var/lib/manual/python3-zstandard.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-zstandard` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (15 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 zstd
```

<details><summary>Full build chain (15)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/fd/aa/3e0508d5a5dd96529cdc5a97011299056e14c6505b678fd58938792794b1/zstandard-0.25.0.tar.gz
mkdir -p dist src/python3-zstandard
curl -fL -o dist/$(basename $URL) $URL
echo "7713e1179d162cf5c7906da876ec2ccb9c3a9dcbdffef0cc7f70c3667a205f0b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-zstandard --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-zstandard; B=build/$QNX_ARCH/python3-zstandard; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=77"; }
```

```bash
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" \
  qnx_py_wheel -x -C--build-option=--system-zstd
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- python-zstandard: Zstandard compression for Python (one-shot, streaming, dictionaries). Its C backend, cross compiled like netifaces (scripts/lib/pywheel.sh), linked against the ported zstd library (--system-zstd) instead of the copy bundled in the sdist. The optional cffi backend is not built: the build's requirement check (cffi>=2 on the build machine) is skipped (-x), and without cffi setup.py leaves that backend out.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-zstandard`.

### 5. What it installs

10 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-zstandard` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-zstandard
ls dists/8.0/*/ | grep '^python3-zstandard-'
```

This writes `python3-zstandard-<ver>-r0.tar.gz` (run time) and `python3-zstandard-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-zstandard
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-zstandard-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-zstandard-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_msgpack_zstandard`](../../tests/src/) links python3-zstandard the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-msgpack-zstandard`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-msgpack-zstandard
/usr/local/bin/qnx-port-tests/test_python3_msgpack_zstandard
```

```
== python3-zstandard ==
  ok   ...
PASS python3-zstandard
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_msgpack_zstandard` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_msgpack_zstandard`](../../tests/src/) -- a program linked against python3-zstandard the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-msgpack 1.2.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-msgpack-1.2.4-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-zstandard 0.25.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-zstandard-0.25.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-msgpack 1.2.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-msgpack-1.2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-zstandard 0.25.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-zstandard-0.25.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.25.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-zstandard=<version>`), sources in `ported/python3-zstandard/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-zstandard`; port one with `scripts/port-update python3-zstandard`.

<details><summary>recipe.sh</summary>

```bash
# python-zstandard: Zstandard compression for Python (one-shot, streaming,
# dictionaries). Its C backend, cross compiled like netifaces
# (scripts/lib/pywheel.sh), linked against the ported zstd library
# (--system-zstd) instead of the copy bundled in the sdist. The optional cffi
# backend is not built: the build's requirement check (cffi>=2 on the build
# machine) is skipped (-x), and without cffi setup.py leaves that backend out.
VERSION=0.25.0
TARBALL=https://files.pythonhosted.org/packages/fd/aa/3e0508d5a5dd96529cdc5a97011299056e14c6505b678fd58938792794b1/zstandard-0.25.0.tar.gz
SHA256=7713e1179d162cf5c7906da876ec2ccb9c3a9dcbdffef0cc7f70c3667a205f0b
DEPS="python3 zstd"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" \
  qnx_py_wheel -x -C--build-option=--system-zstd
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
