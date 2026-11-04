# cython 3.3.1

> Cython: compiles Python and Cython code to C extension modules

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/99/ca/18e324972e5cac914816fdd7bb03e19a455b90039c277a538f7781b50c85/cython-3.3.1.tar.gz |
| SHA-256 | `e0e45989e821bef6aa9ae15c7d583ab5e055f7406c4b3624e49e06d24e90375a` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cython`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `cython`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`cython`](https://openqnx.github.io/oss/package/x86_64/cython.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/cython.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add cython` (dependencies come along); one version: `apk add cython=<version>` | `apk del cython` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install cython` (dependencies first) | `oss-get remove cython` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-cython && /usr/local/bin/qnx-port-tests/test_cython    # its on-target test, from tests-8.0-<arch>
apk del cython qnx-test-cython
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/cython.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/cython.list
rm var/lib/manual/cython.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cython` performs exactly the same steps (2–5) in one command.

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
URL=https://files.pythonhosted.org/packages/99/ca/18e324972e5cac914816fdd7bb03e19a455b90039c277a538f7781b50c85/cython-3.3.1.tar.gz
mkdir -p dist src/cython
curl -fL -o dist/$(basename $URL) $URL
echo "e0e45989e821bef6aa9ae15c7d583ab5e055f7406c4b3624e49e06d24e90375a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/cython --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/cython; B=build/$QNX_ARCH/cython; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=77"; }
```

```bash
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
```

```bash
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Cython: the Python-to-C compiler for extension modules. Built from the source distribution with its compiled modules (the compiler's scanner, parser and code generator, Cython.Runtime.refnanny, ...), cross compiled for the target's python3 (scripts/lib/pywheel.sh); the cython, cythonize and cygdb commands are written as launchers.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cython`.

### 5. What it installs

343 files under `/usr/local`, 3 programs.

```
/usr/local/bin/cygdb
/usr/local/bin/cython
/usr/local/bin/cythonize
```

The complete list is `stage/<arch>/.manifest/cython` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cython
ls dists/8.0/*/ | grep '^cython-'
```

This writes `cython-<ver>-r0.tar.gz` (run time) and `cython-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cython
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cython-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cython-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cython`](../../tests/src/) links cython the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cython`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cython
/usr/local/bin/qnx-port-tests/test_cython
```

```
== cython ==
  ok   ...
PASS cython
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cython` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cython`](../../tests/src/) -- a program linked against cython the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - temporary directory
   - module written as qnxcy.so
   - python3 imports the Cython-compiled module and every check in it passes
   - cython --version reports Cython 3.x
   - cython compiles tiny.pyx to C on the target (tiny.c defines PyInit_tiny)

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> cython`.

## Versions

This recipe builds **3.3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cython=<version>`), sources in `ported/cython/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cython`; port one with `scripts/port-update cython`.

<details><summary>recipe.sh</summary>

```bash
# Cython: the Python-to-C compiler for extension modules. Built from the
# source distribution with its compiled modules (the compiler's scanner,
# parser and code generator, Cython.Runtime.refnanny, ...), cross compiled
# for the target's python3 (scripts/lib/pywheel.sh); the cython, cythonize
# and cygdb commands are written as launchers.
VERSION=3.3.1
TARBALL=https://files.pythonhosted.org/packages/99/ca/18e324972e5cac914816fdd7bb03e19a455b90039c277a538f7781b50c85/cython-3.3.1.tar.gz
SHA256=e0e45989e821bef6aa9ae15c7d583ab5e055f7406c4b3624e49e06d24e90375a
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_sdist; }
do_configure() { qnx_py_venv "setuptools>=77"; }
do_build() {
  rm -rf "$S/build"
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install && qnx_py_scripts "$B"/wheel/*.whl; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
