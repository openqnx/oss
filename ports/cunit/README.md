# cunit 2.1-3

> Unit testing framework for C

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2 |
| SHA-256 | `f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cunit`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add cunit
apk add cunit-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-cunit && /usr/local/bin/qnx-port-tests/test_cunit    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install cunit`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild cunit` performs exactly the same steps (2–5) in one command.

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
URL=https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2
mkdir -p dist src/cunit
curl -fL -o dist/$(basename $URL) $URL
echo "f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/cunit --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/cunit < ports/cunit/patches/0001-drop-ansi-flag.patch
```

**[`0001-drop-ansi-flag.patch`](patches/0001-drop-ansi-flag.patch)** — Do not force -ansi.

> configure.in adds "-Wall -W -pedantic -Wshadow -ansi" to CFLAGS. qcc, the
> QNX compiler driver, rejects -ansi outright ("cc: -ansi no longer
> supported"), so configure concludes the C compiler cannot create
> executables. The same line already asks for -std=c99, which is the standard
> the code is written to; drop the contradictory -ansi.

Touches: `configure.in` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/cunit; B=build/$QNX_ARCH/cunit; mkdir -p $B
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --enable-shared --disable-static --disable-console
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The 2.1-3 tarball ships configure.in without configure, and a source tree autoconf considers "already configured" -- an out-of-tree build is refused. Build in a copy of the tree instead.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH cunit`.

### 5. What it installs

40 files under `/usr/local`: 2 shared-library entries, 9 headers.

```
/usr/local/lib/libcunit.so
/usr/local/lib/libcunit.so.1
```

The complete list is `stage/<arch>/.manifest/cunit` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa cunit
ls dists/8.0/*/ | grep '^cunit-'
```

This writes `cunit-<ver>-r0.tar.gz` (run time) and `cunit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add cunit
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/cunit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/cunit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cunit`](../../tests/src/) links cunit the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cunit`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cunit
/usr/local/bin/qnx-port-tests/test_cunit
```

```
== cunit ==
  ok   ...
PASS cunit
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cunit` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cunit`](../../tests/src/) -- a program linked against cunit the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - registry initialised
   - two suites added
   - four tests added
   - test without a function rejected
   - 3 tests run, 1 failed (suite with failing init skipped)
   - 6 assertions evaluated
   - 1 suite run, 1 suite failed to initialise
   - suite cleanup ran
   - failure record names the test, line and condition
   - lookup by name
   - registry cleaned up

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | cunit 2.1.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/cunit-2.1.3-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | cunit 2.1.3-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/cunit-2.1.3-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.1-3**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add cunit=<version>`), sources in `ported/cunit/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates cunit`; port one with `scripts/port-update cunit`.

<details><summary>recipe.sh</summary>

```bash
VERSION=2.1-3
TARBALL=https://downloads.sourceforge.net/project/cunit/CUnit/2.1-3/CUnit-2.1-3.tar.bz2
SHA256=f5b29137f845bb08b77ec60584fdb728b4e58f1023e6f249a464efa49a40f214
BUILDSYS=custom
# The 2.1-3 tarball ships configure.in without configure, and a source tree
# autoconf considers "already configured" -- an out-of-tree build is refused.
# Build in a copy of the tree instead.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && autoreconf -fi && \
  CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" \
     --enable-shared --disable-static --disable-console
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
