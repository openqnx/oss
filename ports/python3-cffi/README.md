# python3-cffi 2.1.1

> C foreign function interface for Python (ABI and API mode, on libffi)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/9e/ef/008a1939e372c06329a3fce4279c02f328488f3526744906eeec3da7ad5f/cffi-2.1.1.tar.gz |
| SHA-256 | `dd31f52ea1086513bb9df30f8fcee9b8918323ae067a3d5b78bc826a000712be` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) [`libffi`](../libffi/) [`python3-cparser`](../python3-cparser/) |
| Run-time dependencies | [`libffi`](../libffi/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_cffi`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-cffi`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-cffi`](https://openqnx.github.io/oss/package/x86_64/python3-cffi.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-cffi.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-cffi` (dependencies come along); one version: `apk add python3-cffi=<version>` | `apk del python3-cffi` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-cffi` (dependencies first) | `oss-get remove python3-cffi` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-cffi && /usr/local/bin/qnx-port-tests/test_python3_cffi    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-cffi qnx-test-python3-cffi
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-cffi.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-cffi.list
rm var/lib/manual/python3-cffi.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-cffi` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (16 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 libffi python3-cparser
```

<details><summary>Full build chain (16)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-cparser`](../python3-cparser/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/9e/ef/008a1939e372c06329a3fce4279c02f328488f3526744906eeec3da7ad5f/cffi-2.1.1.tar.gz
mkdir -p dist src/python3-cffi
curl -fL -o dist/$(basename $URL) $URL
echo "dd31f52ea1086513bb9df30f8fcee9b8918323ae067a3d5b78bc826a000712be  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-cffi --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-cffi; B=build/$QNX_ARCH/python3-cffi; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=77"; }
```

```bash
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" qnx_py_wheel
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- cffi: the C foreign function interface for Python -- calls C code from Python in ABI mode (dlopen, no compiler) or API mode (generated extension modules). Its backend _cffi_backend is a setuptools C extension on libffi, cross compiled like netifaces (scripts/lib/pywheel.sh); setup.py would ask pkg-config of the build machine, so libffi's flags are given here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-cffi`.

### 5. What it installs

31 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-cffi` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-cffi
ls dists/8.0/*/ | grep '^python3-cffi-'
```

This writes `python3-cffi-<ver>-r0.tar.gz` (run time) and `python3-cffi-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-cffi
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-cffi-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-cffi-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_cffi`](../../tests/src/) links python3-cffi the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-cffi`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-cffi
/usr/local/bin/qnx-port-tests/test_python3_cffi
```

```
== python3-cffi ==
  ok   ...
PASS python3-cffi
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_cffi` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_cffi`](../../tests/src/) -- a program linked against python3-cffi the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> python3-cffi`.

## Versions

This recipe builds **2.1.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-cffi=<version>`), sources in `ported/python3-cffi/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-cffi`; port one with `scripts/port-update python3-cffi`.

<details><summary>recipe.sh</summary>

```bash
# cffi: the C foreign function interface for Python -- calls C code from
# Python in ABI mode (dlopen, no compiler) or API mode (generated extension
# modules). Its backend _cffi_backend is a setuptools C extension on libffi,
# cross compiled like netifaces (scripts/lib/pywheel.sh); setup.py would ask
# pkg-config of the build machine, so libffi's flags are given here.
VERSION=2.1.1
TARBALL=https://files.pythonhosted.org/packages/9e/ef/008a1939e372c06329a3fce4279c02f328488f3526744906eeec3da7ad5f/cffi-2.1.1.tar.gz
SHA256=dd31f52ea1086513bb9df30f8fcee9b8918323ae067a3d5b78bc826a000712be
DEPS="python3 libffi python3-cparser"
RUNTIME_DEPS="python3 python3-cparser"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" qnx_py_wheel
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
