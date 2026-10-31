# p11-kit 0.25.5

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/p11-glue/p11-kit.git (tag `0.25.5`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`libffi`](../libffi/) [`libtasn1`](../libtasn1/) |
| Run-time dependencies | [`libffi`](../libffi/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_p11kit`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add p11-kit
apk add p11-kit-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-p11kit && /usr/local/bin/qnx-port-tests/test_p11kit    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install p11-kit`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild p11-kit` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libffi libtasn1
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b 0.25.5 --recurse-submodules --shallow-submodules \
    https://github.com/p11-glue/p11-kit.git src/p11-kit
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/p11-kit src/p11-kit \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dtrust_module=disabled \
      -Dsystemd=disabled \
      -Dbash_completion=disabled \
      -Dgtk_doc=false \
      -Dman=false \
      -Dnls=false \
      -Dtest=false \
      -Dlibffi=enabled
ninja -C build/$QNX_ARCH/p11-kit
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/p11-kit install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH p11-kit`.

### 5. What it installs

19 files under `/usr/local`: 4 shared-library entries, 1 programs, 9 headers.

```
/usr/local/lib/libp11-kit.so
/usr/local/lib/libp11-kit.so.0
/usr/local/lib/libp11-kit.so.0.4.1
/usr/local/lib/p11-kit-proxy.so
/usr/local/bin/p11-kit
```

The complete list is `stage/<arch>/.manifest/p11-kit` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa p11-kit
ls dists/8.0/*/ | grep '^p11-kit-'
```

This writes `p11-kit-<ver>-r0.tar.gz` (run time) and `p11-kit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add p11-kit
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/p11-kit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/p11-kit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_p11kit`](../../tests/src/) links p11-kit the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-p11kit`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-p11kit
/usr/local/bin/qnx-port-tests/test_p11kit
```

```
== p11-kit ==
  ok   ...
PASS p11-kit
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_p11kit` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_p11kit`](../../tests/src/) -- a program linked against p11-kit the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - p11_kit_uri_new
   - p11_kit_uri_parse
   - token label decoded from percent-encoding
   - object label attribute
   - p11_kit_uri_format
   - a malformed URI is rejected
   - p11_kit_modules_load_and_initialize
   - the empty case is reported cleanly

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | p11-kit 0.25.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/p11-kit-0.25.5-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | p11-kit 0.25.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/p11-kit-0.25.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.25.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add p11-kit=<version>`), sources in `ported/p11-kit/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates p11-kit`; port one with `scripts/port-update p11-kit`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.25.5; GIT=https://github.com/p11-glue/p11-kit.git; REF=0.25.5
DEPS="libffi libtasn1"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dtrust_module=disabled -Dsystemd=disabled -Dbash_completion=disabled
                   -Dgtk_doc=false -Dman=false -Dnls=false -Dtest=false
                   -Dlibffi=enabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
