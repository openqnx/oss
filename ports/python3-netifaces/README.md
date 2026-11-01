# python3-netifaces 0.11.0

> Portable network interface information for Python

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/a6/91/86a6eac449ddfae239e93ffc1918cf33fd9bab35c04d1e963b311e347a73/netifaces-0.11.0.tar.gz |
| SHA-256 | `043a79146eb2907edf439899f262b3dfe41717d34124298ed281139a8b93ca32` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_netifaces`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-netifaces`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-netifaces`](https://openqnx.github.io/oss/package/x86_64/python3-netifaces.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-netifaces.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-netifaces` (dependencies come along); one version: `apk add python3-netifaces=<version>` | `apk del python3-netifaces` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-netifaces` (dependencies first) | `oss-get remove python3-netifaces` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-netifaces && /usr/local/bin/qnx-port-tests/test_python3_netifaces    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-netifaces qnx-test-python3-netifaces
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-netifaces.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-netifaces.list
rm var/lib/manual/python3-netifaces.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-netifaces` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild python3
```

<details><summary>Full build chain (15)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/a6/91/86a6eac449ddfae239e93ffc1918cf33fd9bab35c04d1e963b311e347a73/netifaces-0.11.0.tar.gz
mkdir -p dist src/python3-netifaces
curl -fL -o dist/$(basename $URL) $URL
echo "043a79146eb2907edf439899f262b3dfe41717d34124298ed281139a8b93ca32  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-netifaces --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/python3-netifaces < ports/python3-netifaces/patches/0001-qnx-libsocket.patch
```

**[`0001-qnx-libsocket.patch`](patches/0001-qnx-libsocket.patch)** — Link libsocket on QNX, in the module and in setup.py's probes.

> QNX keeps getifaddrs(), getnameinfo() and the socket calls in libsocket, not
> in libc. setup.py's probes link test programs without it, so on QNX each
> probe failed and netifaces.c stopped with "You need to add code for your
> platform"; the module itself would also miss its NEEDED libsocket. Link
> libsocket on QNX (taken from sysconfig's platform, since sys.platform is
> the build machine's when cross compiling), and let the probes link the
> same libraries as the module.

Touches: `setup.py` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-netifaces; B=build/$QNX_ARCH/python3-netifaces; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=77"; }
```

```bash
do_build() {
  rm -rf "$S/build"   # setup.py caches its probe results in the build tree
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  qnx_py_wheel
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- netifaces: network interface addresses (netifaces.interfaces(), ifaddresses(), gateways()) for Python. A setuptools C extension, cross compiled like Pillow (scripts/lib/pywheel.sh). Its setup.py probes the platform by compiling and linking small test programs (getifaddrs, getnameinfo, sa_len, routing sockets): QNX keeps all of these in libsocket (patches/0001).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-netifaces`.

### 5. What it installs

7 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-netifaces` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-netifaces
ls dists/8.0/*/ | grep '^python3-netifaces-'
```

This writes `python3-netifaces-<ver>-r0.tar.gz` (run time) and `python3-netifaces-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-netifaces
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-netifaces-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-netifaces-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_netifaces`](../../tests/src/) links python3-netifaces the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-netifaces`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-netifaces
/usr/local/bin/qnx-port-tests/test_python3_netifaces
```

```
== python3-netifaces ==
  ok   ...
PASS python3-netifaces
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_netifaces` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_netifaces`](../../tests/src/) -- a program linked against python3-netifaces the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-netifaces 0.11.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-netifaces-0.11.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-netifaces 0.11.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-netifaces-0.11.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.11.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-netifaces=<version>`), sources in `ported/python3-netifaces/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-netifaces`; port one with `scripts/port-update python3-netifaces`.

<details><summary>recipe.sh</summary>

```bash
# netifaces: network interface addresses (netifaces.interfaces(),
# ifaddresses(), gateways()) for Python. A setuptools C extension, cross
# compiled like Pillow (scripts/lib/pywheel.sh). Its setup.py probes the
# platform by compiling and linking small test programs (getifaddrs,
# getnameinfo, sa_len, routing sockets): QNX keeps all of these in
# libsocket (patches/0001).
VERSION=0.11.0
TARBALL=https://files.pythonhosted.org/packages/a6/91/86a6eac449ddfae239e93ffc1918cf33fd9bab35c04d1e963b311e347a73/netifaces-0.11.0.tar.gz
SHA256=043a79146eb2907edf439899f262b3dfe41717d34124298ed281139a8b93ca32
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"   # setup.py caches its probe results in the build tree
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" \
  qnx_py_wheel
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
