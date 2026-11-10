# python3-pyproject-hooks 1.3.3

> Calls PEP 517 build backend hooks

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/85/11/044d1ae1b4ec0d7af88ee5bc91e081be1022533032b906a7bdabdbb60977/pyproject_hooks-1.3.3-py3-none-any.whl |
| SHA-256 | `5fc53fdac9f7bd63fbcdc868fb5f90b4784d78a53a3d3388cd738b807441a20b` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_pybuild`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `python3-pyproject-hooks`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`python3-pyproject-hooks`](https://openqnx.github.io/oss/package/x86_64/python3-pyproject-hooks.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/python3-pyproject-hooks.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add python3-pyproject-hooks` (dependencies come along); one version: `apk add python3-pyproject-hooks=<version>` | `apk del python3-pyproject-hooks` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install python3-pyproject-hooks` (dependencies first) | `oss-get remove python3-pyproject-hooks` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-python3-pybuild && /usr/local/bin/qnx-port-tests/test_python3_pybuild    # its on-target test, from tests-8.0-<arch>-m-z
apk del python3-pyproject-hooks qnx-test-python3-pybuild
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/python3-pyproject-hooks.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/python3-pyproject-hooks.list
rm var/lib/manual/python3-pyproject-hooks.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-pyproject-hooks` performs exactly the same steps (2–5) in one command.

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
URL=https://files.pythonhosted.org/packages/85/11/044d1ae1b4ec0d7af88ee5bc91e081be1022533032b906a7bdabdbb60977/pyproject_hooks-1.3.3-py3-none-any.whl
mkdir -p dist src/python3-pyproject-hooks
curl -fL -o dist/$(basename $URL) $URL
echo "5fc53fdac9f7bd63fbcdc868fb5f90b4784d78a53a3d3388cd738b807441a20b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-pyproject-hooks --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-pyproject-hooks; B=build/$QNX_ARCH/python3-pyproject-hooks; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pyproject-hooks: calls PEP 517 build backend hooks. Pure Python: the published wheel is platform independent, so it is installed as released (see scripts/lib/pywheel.sh).
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-pyproject-hooks`.

### 5. What it installs

9 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-pyproject-hooks` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-pyproject-hooks
ls dists/8.0/*/ | grep '^python3-pyproject-hooks-'
```

This writes `python3-pyproject-hooks-<ver>-r0.tar.gz` (run time) and `python3-pyproject-hooks-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-pyproject-hooks
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-pyproject-hooks-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-pyproject-hooks-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_pybuild`](../../tests/src/) links python3-pyproject-hooks the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-pybuild`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-pybuild
/usr/local/bin/qnx-port-tests/test_python3_pybuild
```

```
== python3-pyproject-hooks ==
  ok   ...
PASS python3-pyproject-hooks
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_pybuild` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_pybuild`](../../tests/src/) -- a program linked against python3-pyproject-hooks the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-build 1.6.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-build-1.6.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-calver 2025.10.20-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-calver-2025.10.20-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-editables 0.6-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-editables-0.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-flit-core 4.2.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-flit-core-4.2.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-gpep517 23-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-gpep517-23-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-hatch-fancy-pypi-readme 25.1.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-hatch-fancy-pypi-readme-25.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-hatch-vcs 0.5.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-hatch-vcs-0.5.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-hatchling 1.31.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-hatchling-1.31.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-installer 1.1.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-installer-1.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-meson-python 0.22.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-meson-python-0.22.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pyproject-hooks 1.3.3-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-pyproject-hooks-1.3.3-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pyproject-metadata 0.12.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-pyproject-metadata-0.12.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-scikit-build-core 1.1.2-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-scikit-build-core-1.1.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-setuptools_scm 9.2.2-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/python3-setuptools_scm-9.2.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-build 1.6.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-build-1.6.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-calver 2025.10.20-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-calver-2025.10.20-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-editables 0.6-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-editables-0.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-flit-core 4.2.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-flit-core-4.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-gpep517 23-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-gpep517-23-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-hatch-fancy-pypi-readme 25.1.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-hatch-fancy-pypi-readme-25.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-hatch-vcs 0.5.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-hatch-vcs-0.5.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-hatchling 1.31.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-hatchling-1.31.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-installer 1.1.0-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-installer-1.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-meson-python 0.22.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-meson-python-0.22.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pyproject-hooks 1.3.3-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-pyproject-hooks-1.3.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pyproject-metadata 0.12.1-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-pyproject-metadata-0.12.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-scikit-build-core 1.1.2-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-scikit-build-core-1.1.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-setuptools_scm 9.2.2-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/python3-setuptools_scm-9.2.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.3.3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-pyproject-hooks=<version>`), sources in `ported/python3-pyproject-hooks/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-pyproject-hooks`; port one with `scripts/port-update python3-pyproject-hooks`.

<details><summary>recipe.sh</summary>

```bash
# pyproject-hooks: calls PEP 517 build backend hooks.
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.3.3
TARBALL=https://files.pythonhosted.org/packages/85/11/044d1ae1b4ec0d7af88ee5bc91e081be1022533032b906a7bdabdbb60977/pyproject_hooks-1.3.3-py3-none-any.whl
SHA256=5fc53fdac9f7bd63fbcdc868fb5f90b4784d78a53a3d3388cd738b807441a20b
DEPS="python3"
# Python modules are imported, not linked: what they need is named here.
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
