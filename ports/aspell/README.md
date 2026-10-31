# aspell 0.60.8.1

> GNU Aspell spell checker and library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/GNUAspell/aspell.git (tag `rel-0.60.8.1`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_aspell`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add aspell
apk add aspell-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-aspell && /usr/local/bin/qnx-port-tests/test_aspell    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install aspell`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild aspell` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b rel-0.60.8.1 --recurse-submodules --shallow-submodules \
    https://github.com/GNUAspell/aspell.git src/aspell
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
# A git checkout has no ./configure yet
( cd src/aspell && if [ -f autogen.sh ]; then NOCONFIGURE=1 ./autogen.sh; else autoreconf -fi; fi )
mkdir -p build/$QNX_ARCH/aspell && cd build/$QNX_ARCH/aspell
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/aspell/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-nls \
    --enable-compile-in-filters
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH aspell`.

### 5. What it installs

92 files under `/usr/local`: 4 shared-library entries, 9 programs, 2 headers.

```
/usr/local/lib/libaspell.so
/usr/local/lib/libaspell.so.18
/usr/local/lib/libpspell.so
/usr/local/lib/libpspell.so.18
/usr/local/bin/aspell
/usr/local/bin/aspell-import
/usr/local/bin/precat
/usr/local/bin/preunzip
/usr/local/bin/prezip
/usr/local/bin/prezip-bin
/usr/local/bin/pspell-config
/usr/local/bin/run-with-aspell
/usr/local/bin/word-list-compress
```

The complete list is `stage/<arch>/.manifest/aspell` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa aspell
ls dists/8.0/*/ | grep '^aspell-'
```

This writes `aspell-<ver>-r0.tar.gz` (run time) and `aspell-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add aspell
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/aspell-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/aspell-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_aspell`](../../tests/src/) links aspell the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-aspell`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-aspell
/usr/local/bin/qnx-port-tests/test_aspell
```

```
== aspell ==
  ok   ...
PASS aspell
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_aspell` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_aspell`](../../tests/src/) -- a program linked against aspell the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - mkdtemp
   - aspell --version: 0.60.8.1
   - aspell create master: xx.rws compiled
   - new_aspell_speller
   - new_aspell_speller on the compiled dictionary
   - speller language is xx
   - check(\"banana\") == correct
   - check(\"microkernel\") == correct
   - check(\"bananna\") == misspelled
   - check(\"linux\") == unknown
   - suggest(\"bananna\") -> \"banana\" first
   - add_to_session(\"photon\")
   - session word now correct
   - aspell list: exactly bananna, qnz
   - aspell -a: '& cherri ... : cherry'
   - aspell dump master: 7 words

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aspell 0.60.8.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/aspell-0.60.8.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | aspell 0.60.8.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/aspell-0.60.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aspell 0.60.8.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/aspell-0.60.8.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | aspell 0.60.8.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/aspell-0.60.8.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.60.8.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add aspell=<version>`), sources in `ported/aspell/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates aspell`; port one with `scripts/port-update aspell`.

<details><summary>recipe.sh</summary>

```bash
VERSION=0.60.8.1; GIT=https://github.com/GNUAspell/aspell.git; REF=rel-0.60.8.1
DEPS="ncurses"   # configure links -lncurses(w); undeclared, it found the SDP's
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-nls
                  --enable-compile-in-filters )
do_patch() { cd "$SRCROOT" && ./autogen >/dev/null 2>&1 || autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
