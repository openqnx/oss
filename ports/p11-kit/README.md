# p11-kit 0.25.5

> Library and proxy module for loading and sharing PKCS#11 modules

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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `p11-kit` (programs, shared libraries, run-time data) and `p11-kit-dev` (headers, pkg-config/CMake files, static libraries; needs `p11-kit`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`p11-kit`](https://openqnx.github.io/oss/package/x86_64/p11-kit.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/p11-kit.html)) · [`p11-kit-dev`](https://openqnx.github.io/oss/package/x86_64/p11-kit-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/p11-kit-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add p11-kit` (dependencies come along); one version: `apk add p11-kit=<version>` | `apk del p11-kit` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install p11-kit` (dependencies first) | `oss-get remove p11-kit` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`p11-kit-dev` the same way, by its own name (`apk add p11-kit-dev`, `oss-get install p11-kit-dev`, by hand with its own file and list `p11-kit-dev.list`); remove it before `p11-kit`, which it needs.

```sh
# on the QNX target, as root
apk add p11-kit p11-kit-dev
apk add qnx-test-p11kit && /usr/local/bin/qnx-port-tests/test_p11kit    # its on-target test, from tests-8.0-<arch>-m-z
apk del p11-kit p11-kit-dev qnx-test-p11kit
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/p11-kit.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/p11-kit.list
rm var/lib/manual/p11-kit.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

[`test_p11kit`](../../tests/src/) links p11-kit the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-p11kit`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | p11-kit 0.25.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/p11-kit-0.25.5-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | p11-kit 0.25.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/p11-kit-0.25.5-r0.log) |

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
