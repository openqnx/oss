# python3-pluggy 1.7.0

> Plugin and hook calling mechanism for Python (used by pytest)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/40/9e/2b38731e0fc536806f16490e1a12d7f0dc2a1235aa8cc07bcc75416a7daa/pluggy-1.7.0-py3-none-any.whl |
| SHA-256 | `7dd7b0d8832ba3cb632c306926ded123429211b83641b35dc5c41ad2d34f9bec` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pymods`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add python3-pluggy
apk add python3-pluggy-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-pymods && /usr/local/bin/qnx-port-tests/test_pymods    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install python3-pluggy`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-pluggy` performs exactly the same steps (2–5) in one command.

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
URL=https://files.pythonhosted.org/packages/40/9e/2b38731e0fc536806f16490e1a12d7f0dc2a1235aa8cc07bcc75416a7daa/pluggy-1.7.0-py3-none-any.whl
mkdir -p dist src/python3-pluggy
curl -fL -o dist/$(basename $URL) $URL
echo "7dd7b0d8832ba3cb632c306926ded123429211b83641b35dc5c41ad2d34f9bec  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-pluggy --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-pluggy; B=build/$QNX_ARCH/python3-pluggy; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- pluggy: plugin and hook calling (pytest's plugin system). Pure Python: the published wheel is platform independent, so it is installed as released (see scripts/lib/pywheel.sh).
- Python modules are imported, not linked: what they need is named here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-pluggy`.

### 5. What it installs

19 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-pluggy` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-pluggy
ls dists/8.0/*/ | grep '^python3-pluggy-'
```

This writes `python3-pluggy-<ver>-r0.tar.gz` (run time) and `python3-pluggy-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-pluggy
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-pluggy-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-pluggy-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pymods`](../../tests/src/) links python3-pluggy the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pymods`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pymods
/usr/local/bin/qnx-port-tests/test_pymods
```

```
== python3-pluggy ==
  ok   ...
PASS python3-pluggy
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pymods` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pymods`](../../tests/src/) -- a program linked against python3-pluggy the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-astor 0.8.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-astor-0.8.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-async-generator 1.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-async-generator-1.10-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-click 8.5.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-click-8.5.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-coverage 7.17.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-coverage-7.17.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-curio 1.6_git20251221-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-curio-1.6_git20251221-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-idna 3.20-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-idna-3.20-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-iniconfig 2.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-iniconfig-2.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-mypy-extensions 1.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-mypy-extensions-1.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pluggy 1.7.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pluggy-1.7.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pretend 1.0.9-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pretend-1.0.9-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pygments 2.21.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pygments-2.21.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pytest 9.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pytest-9.2.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pytest-cov 7.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pytest-cov-7.1.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pytest-tornasync 0.6.0_p2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pytest-tornasync-0.6.0_p2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-pytokens 0.4.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-pytokens-0.4.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-sniffio 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-sniffio-1.3.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-sortedcontainers 2.4.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-sortedcontainers-2.4.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-tornado 6.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-tornado-6.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-traitlets 5.16.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-traitlets-5.16.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-versioneer 0.29-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-versioneer-0.29-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-astor 0.8.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-astor-0.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-async-generator 1.10-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-async-generator-1.10-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-click 8.5.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-click-8.5.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-coverage 7.17.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-coverage-7.17.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-curio 1.6_git20251221-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-curio-1.6_git20251221-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-idna 3.20-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-idna-3.20-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-iniconfig 2.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-iniconfig-2.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-mypy-extensions 1.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-mypy-extensions-1.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pluggy 1.7.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pluggy-1.7.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pretend 1.0.9-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pretend-1.0.9-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pygments 2.21.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pygments-2.21.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pytest 9.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pytest-9.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pytest-cov 7.1.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pytest-cov-7.1.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pytest-tornasync 0.6.0_p2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pytest-tornasync-0.6.0_p2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-pytokens 0.4.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-pytokens-0.4.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-sniffio 1.3.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-sniffio-1.3.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-sortedcontainers 2.4.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-sortedcontainers-2.4.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-tornado 6.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-tornado-6.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-traitlets 5.16.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-traitlets-5.16.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-versioneer 0.29-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-versioneer-0.29-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.7.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-pluggy=<version>`), sources in `ported/python3-pluggy/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-pluggy`; port one with `scripts/port-update python3-pluggy`.

<details><summary>recipe.sh</summary>

```bash
# pluggy: plugin and hook calling (pytest's plugin system).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=1.7.0
TARBALL=https://files.pythonhosted.org/packages/40/9e/2b38731e0fc536806f16490e1a12d7f0dc2a1235aa8cc07bcc75416a7daa/pluggy-1.7.0-py3-none-any.whl
SHA256=7dd7b0d8832ba3cb632c306926ded123429211b83641b35dc5c41ad2d34f9bec
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
