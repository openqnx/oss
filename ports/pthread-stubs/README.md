# pthread-stubs 0.5

> pkg-config file for pthread symbols needed by X libraries

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz |
| SHA-256 | `59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_pthread_stubs`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add pthread-stubs
apk add pthread-stubs-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-pthread-stubs && /usr/local/bin/qnx-port-tests/test_pthread_stubs    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install pthread-stubs`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild pthread-stubs` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz
mkdir -p dist src/pthread-stubs
curl -fL -o dist/$(basename $URL) $URL
echo "59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/pthread-stubs --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/pthread-stubs && cd build/$QNX_ARCH/pthread-stubs
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/pthread-stubs/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The gitlab.freedesktop.org repo for this one is not publicly clonable (git asks for credentials); the release tarball is.
- Pure .pc file. It exists so that libxcb can say "I need pthreads" portably: on platforms where libc already provides them -- QNX among them -- the package resolves to no library at all, which is exactly the right answer here.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH pthread-stubs`.

### 5. What it installs

1 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/pthread-stubs` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa pthread-stubs
ls dists/8.0/*/ | grep '^pthread-stubs-'
```

This writes `pthread-stubs-<ver>-r0.tar.gz` (run time) and `pthread-stubs-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add pthread-stubs
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/pthread-stubs-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/pthread-stubs-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_pthread_stubs`](../../tests/src/) links pthread-stubs the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-pthread-stubs`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-pthread-stubs
/usr/local/bin/qnx-port-tests/test_pthread_stubs
```

```
== pthread-stubs ==
  ok   ...
PASS pthread-stubs
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_pthread_stubs` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_pthread_stubs`](../../tests/src/) -- a program linked against pthread-stubs the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - pthread_key_create
   - pthread_barrier_init(4)
   - 4 threads created
   - condition variable: all 4 signalled
   - each thread saw its own thread-specific value
   - mutex: 4 x 10000 increments = 40000
   - pthread_once ran once
   - rwlock-protected sum 6
   - key destructor ran for each thread
   - pthread_equal

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | pthread-stubs 0.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/pthread-stubs-0.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | pthread-stubs 0.5-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/pthread-stubs-0.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add pthread-stubs=<version>`), sources in `ported/pthread-stubs/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates pthread-stubs`; port one with `scripts/port-update pthread-stubs`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.5
# The gitlab.freedesktop.org repo for this one is not publicly clonable
# (git asks for credentials); the release tarball is.
TARBALL=https://xorg.freedesktop.org/releases/individual/lib/libpthread-stubs-0.5.tar.xz
SHA256=59da566decceba7c2a7970a4a03b48d9905f1262ff94410a649224e33d2442bc
BUILDSYS=autotools
# Pure .pc file. It exists so that libxcb can say "I need pthreads" portably:
# on platforms where libc already provides them -- QNX among them -- the package
# resolves to no library at all, which is exactly the right answer here.
CONF_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
