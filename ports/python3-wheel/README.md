# python3-wheel 0.46.1

> Command line tool and library for the Python wheel package format

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.pythonhosted.org/packages/af/44/31d964a260fcda6bf2f59384231bbf4962ac796a5d36c554162b5411b8db/wheel-0.46.1-py3-none-any.whl |
| SHA-256 | `f796f65d72750ccde090663e466d0ca37cd72b62870f7520b96d34cdc07d86d8` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`python3`](../python3/) [`python3-packaging`](../python3-packaging/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_python3_wheel`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add python3-wheel
apk add python3-wheel-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-python3-wheel && /usr/local/bin/qnx-port-tests/test_python3_wheel    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install python3-wheel`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild python3-wheel` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild python3 python3-packaging
```

<details><summary>Full build chain (16)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`python3-packaging`](../python3-packaging/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.pythonhosted.org/packages/af/44/31d964a260fcda6bf2f59384231bbf4962ac796a5d36c554162b5411b8db/wheel-0.46.1-py3-none-any.whl
mkdir -p dist src/python3-wheel
curl -fL -o dist/$(basename $URL) $URL
echo "f796f65d72750ccde090663e466d0ca37cd72b62870f7520b96d34cdc07d86d8  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/python3-wheel --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/python3-wheel; B=build/$QNX_ARCH/python3-wheel; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_py_install_pure; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- wheel (version as in the QNX Everywhere 8.0.4r1 image). Pure Python: the published wheel is platform independent, so it is installed as released (see scripts/lib/pywheel.sh).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH python3-wheel`.

### 5. What it installs

17 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/python3-wheel` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa python3-wheel
ls dists/8.0/*/ | grep '^python3-wheel-'
```

This writes `python3-wheel-<ver>-r0.tar.gz` (run time) and `python3-wheel-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add python3-wheel
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/python3-wheel-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/python3-wheel-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_python3_wheel`](../../tests/src/) links python3-wheel the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-python3-wheel`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-python3-wheel
/usr/local/bin/qnx-port-tests/test_python3_wheel
```

```
== python3-wheel ==
  ok   ...
PASS python3-wheel
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_python3_wheel` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_python3_wheel`](../../tests/src/) -- a program linked against python3-wheel the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-wheel 0.46.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/python3-wheel-0.46.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | python3-wheel 0.46.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/python3-wheel-0.46.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-wheel 0.46.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/python3-wheel-0.46.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | python3-wheel 0.46.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/python3-wheel-0.46.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.46.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add python3-wheel=<version>`), sources in `ported/python3-wheel/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates python3-wheel`; port one with `scripts/port-update python3-wheel`.

<details><summary>recipe.sh</summary>

```bash
# wheel (version as in the QNX Everywhere 8.0.4r1 image).
# Pure Python: the published wheel is platform independent, so it is
# installed as released (see scripts/lib/pywheel.sh).
VERSION=0.46.1
TARBALL=https://files.pythonhosted.org/packages/af/44/31d964a260fcda6bf2f59384231bbf4962ac796a5d36c554162b5411b8db/wheel-0.46.1-py3-none-any.whl
SHA256=f796f65d72750ccde090663e466d0ca37cd72b62870f7520b96d34cdc07d86d8
DEPS="python3 python3-packaging"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_fetch() { qnx_py_fetch_wheel; }
do_build() { :; }
do_install() { qnx_py_install_pure; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
