# dbus 1.16.0

> D-Bus message bus daemon and library for interprocess communication

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.freedesktop.org/dbus/dbus.git (tag `dbus-1.16.0`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_dbus`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add dbus
apk add dbus-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-dbus && /usr/local/bin/qnx-port-tests/test_dbus    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install dbus`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild dbus` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild expat
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b dbus-1.16.0 --recurse-submodules --shallow-submodules \
    https://gitlab.freedesktop.org/dbus/dbus.git src/dbus
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/dbus < ports/dbus/patches/0001-qnx-getpeereid-fallback.patch
```

**[`0001-qnx-getpeereid-fallback.patch`](patches/0001-qnx-getpeereid-fallback.patch)** — QNX: take the peer uid from getpeereid() when no SCM_CREDS came (GDBus clients)

Touches: `dbus/dbus-sysdeps-unix.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/dbus src/dbus \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Depoll=disabled \
      -Dsystemd=disabled \
      -Dselinux=disabled \
      -Dapparmor=disabled \
      -Dx11_autolaunch=disabled \
      -Ddoxygen_docs=disabled \
      -Dinotify=disabled \
      -Dkqueue=disabled \
      -Dducktype_docs=disabled \
      -Dxml_docs=disabled \
      -Dmodular_tests=disabled \
      -Dtools=false \
      -Dstats=false
ninja -C build/$QNX_ARCH/dbus
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/dbus install
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH dbus`.

### 5. What it installs

41 files under `/usr/local`: 3 shared-library entries, 1 programs, 17 headers.

```
/usr/local/lib/libdbus-1.so
/usr/local/lib/libdbus-1.so.3
/usr/local/lib/libdbus-1.so.3.38.3
/usr/local/bin/dbus-daemon
```

The complete list is `stage/<arch>/.manifest/dbus` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa dbus
ls dists/8.0/*/ | grep '^dbus-'
```

This writes `dbus-<ver>-r0.tar.gz` (run time) and `dbus-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add dbus
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/dbus-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/dbus-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_dbus`](../../tests/src/) links dbus the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-dbus`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-dbus
/usr/local/bin/qnx-port-tests/test_dbus
```

```
== dbus ==
  ok   ...
PASS dbus
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_dbus` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_dbus`](../../tests/src/) -- a program linked against dbus the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - dbus_message_new_method_call
   - append a string argument
   - append an int32 argument
   - member name
   - message type
   - dbus_message_marshal
   - demarshal_bytes_needed agrees with the marshalled size
   - dbus_message_demarshal
   - dbus_message_get_args
   - arguments survive the round trip
   - bus name validation
   - invalid bus name rejected
   - object path validation
   - unreachable bus reports an error

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dbus 1.16.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/dbus-1.16.0-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dbus 1.16.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/dbus-1.16.0-r1.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | dbus 1.16.0-r3 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/dbus-1.16.0-r3.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dbus 1.16.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/dbus-1.16.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dbus 1.16.0-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/dbus-1.16.0-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | dbus 1.16.0-r3 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/dbus-1.16.0-r3.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.16.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add dbus=<version>`), sources in `ported/dbus/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates dbus`; port one with `scripts/port-update dbus`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.16.0; GIT=https://gitlab.freedesktop.org/dbus/dbus.git; REF=dbus-1.16.0
DEPS="expat"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Depoll=disabled -Dsystemd=disabled -Dselinux=disabled -Dapparmor=disabled
                   -Dx11_autolaunch=disabled -Ddoxygen_docs=disabled -Dinotify=disabled -Dkqueue=disabled
                   -Dducktype_docs=disabled -Dxml_docs=disabled
                   -Dmodular_tests=disabled -Dtools=false -Dstats=false )
# -Dinotify=disabled: QNX's inotify is an emulation through /dev/fsnotify,
# and dbus-daemon's service-directory watcher blocked in it (state REPLY on
# the fsnotify server) while activating a service -- a deadlock: the
# activated service's authentication was never answered (hazard 136). The
# default watcher does nothing; the daemon reads the service directories at
# start and again when a requested service is not in its cache.
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
