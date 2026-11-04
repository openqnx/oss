# python3-virtualenv 20.39.1

> Isolated Python environments

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/f6/7f/89d7e8a397752826dbbb81127dfb8feab8286f4f1f91a820f646f04368fb/virtualenv-20.39.1-py3-none-any.whl |
| SHA-256 | `a00332e0b089ba64edefbf94ef34e6db33f45fe5184df0652cf0b754dcd36190` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) [`python3-distlib`](../python3-distlib/) [`python3-filelock`](../python3-filelock/) [`python3-platformdirs`](../python3-platformdirs/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_virtualenv`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-virtualenv`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-virtualenv`](https://openqnx.github.io/oss/package/x86_64/python3-virtualenv.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-virtualenv.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-virtualenv` (dependencies come along); one version: `apk add python3-virtualenv=<version>` | `apk del python3-virtualenv` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-virtualenv` (dependencies first) | `oss-get remove python3-virtualenv` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-virtualenv && /usr/local/bin/qnx-port-tests/test_python3_virtualenv    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-virtualenv qnx-test-python3-virtualenv
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-virtualenv.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-virtualenv.list
rm var/lib/manual/python3-virtualenv.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-virtualenv` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (18 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3 python3-distlib python3-filelock python3-platformdirs
```

<details><summary>Full build chain (18)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-distlib`](../python3-distlib/) [`python3-filelock`](../python3-filelock/) [`python3-platformdirs`](../python3-platformdirs/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/f6/7f/89d7e8a397752826dbbb81127dfb8feab8286f4f1f91a820f646f04368fb/virtualenv-20.39.1-py3-none-any.whl
mkdir -p dist src/python3-virtualenv
curl -fL -o dist/$(basename $URL) $URL
echo "a00332e0b089ba64edefbf94ef34e6db33f45fe5184df0652cf0b754dcd36190  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-virtualenv --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-virtualenv; B=build/$QNX_ARCH/python3-virtualenv; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- virtualenv: isolated Python environments. Version 20.39.1: virtualenv 21 needs python-discovery and packaging>=26.3. Pure Python: the published wheel is platform independent, so it is installed as released (see scripts/lib/pywheel.sh). Its console scripts are written as launchers (qnx_py_scripts).
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-virtualenv`.

### 5. What it installs

113 files under `/usr/local`, 1 programs.

```
/usr/local/bin/virtualenv
```

The complete list is `stage/<arch>/.manifest/python3-virtualenv` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-virtualenv
ls dists/8.0/*/ | grep '^python3-virtualenv-'
```

This writes `python3-virtualenv-<ver>-r0.tar.gz` (run time) and `python3-virtualenv-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-virtualenv
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-virtualenv-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-virtualenv-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_virtualenv`](../../tests/src/) links python3-virtualenv the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-virtualenv`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-virtualenv
/usr/local/bin/qnx-port-tests/test_python3_virtualenv
```

```
== python3-virtualenv ==
  ok   ...
PASS python3-virtualenv
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_virtualenv` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_virtualenv`](../../tests/src/) -- a program linked against python3-virtualenv the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> python3-virtualenv`.

## Versions

This recipe builds **20.39.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-virtualenv=<version>`), sources in `ported/python3-virtualenv/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-virtualenv`; port one with `scripts/port-update python3-virtualenv`.

<details><summary>recipe.sh</summary>

```bash
# virtualenv: isolated Python environments.
# Version 20.39.1: virtualenv 21 needs python-discovery and packaging>=26.3.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
# Its console scripts are written as launchers (qnx_py_scripts).
VERSION=20.39.1
TARBALL=https://files.pythonhosted.org/packages/f6/7f/89d7e8a397752826dbbb81127dfb8feab8286f4f1f91a820f646f04368fb/virtualenv-20.39.1-py3-none-any.whl
SHA256=a00332e0b089ba64edefbf94ef34e6db33f45fe5184df0652cf0b754dcd36190
DEPS="python3 python3-distlib python3-filelock python3-platformdirs"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3 python3-distlib python3-filelock python3-platformdirs"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure && qnx_py_scripts "$SRCROOT"/*.whl; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
