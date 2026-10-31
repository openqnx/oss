# zsh 5.9

> Z shell

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz |
| SHA-256 | `9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_zsh`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add zsh
apk add zsh-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-zsh && /usr/local/bin/qnx-port-tests/test_zsh    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install zsh`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild zsh` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (5 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses pcre2 libiconv
```

<details><summary>Full build chain (5)</summary>

[`bzip2`](../bzip2/) [`libiconv`](../libiconv/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
mkdir -p dist src/zsh
curl -fL -o dist/$(basename $URL) $URL
echo "9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/zsh --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/zsh < ports/zsh/patches/0001-aports-CODESET.patch
patch -p1 -d src/zsh < ports/zsh/patches/0002-aports-configure-shared.patch
```

**[`0001-aports-CODESET.patch`](patches/0001-aports-CODESET.patch)** — QNX change from QNX's aports (extra/zsh/CODESET.patch).

> Taken from qnx-ports/aports extra/zsh -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `zsh-5.9/Src/Modules/langinfo.c` · Upstream status: QNX-specific

**[`0002-aports-configure-shared.patch`](patches/0002-aports-configure-shared.patch)** — QNX change from QNX's aports (extra/zsh/configure-shared.patch).

> Taken from qnx-ports/aports extra/zsh -- the APKBUILD the QNX Everywhere 8.0.4r1
> image's package is built from.

Touches: `a/configure.ac` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/zsh && cd build/$QNX_ARCH/zsh
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/zsh/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-multibyte \
    --with-tcsetpgrp \
    --disable-gdbm
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Z shell. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH zsh`.

### 5. What it installs

1279 files under `/usr/local`, 2 programs.

```
/usr/local/bin/zsh
/usr/local/bin/zsh-5.9
```

The complete list is `stage/<arch>/.manifest/zsh` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa zsh
ls dists/8.0/*/ | grep '^zsh-'
```

This writes `zsh-<ver>-r0.tar.gz` (run time) and `zsh-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add zsh
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/zsh-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/zsh-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_zsh`](../../tests/src/) links zsh the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-zsh`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-zsh
/usr/local/bin/qnx-port-tests/test_zsh
```

```
== zsh ==
  ok   ...
PASS zsh
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_zsh` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_zsh`](../../tests/src/) -- a program linked against zsh the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zsh 5.9-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/zsh-5.9-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | zsh 5.9-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/zsh-5.9-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zsh 5.9-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/zsh-5.9-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | zsh 5.9-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/zsh-5.9-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.9**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add zsh=<version>`), sources in `ported/zsh/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates zsh`; port one with `scripts/port-update zsh`.

<details><summary>recipe.sh</summary>

```bash
# Z shell. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=5.9
TARBALL=https://downloads.sourceforge.net/project/zsh/zsh/5.9/zsh-5.9.tar.xz
SHA256=9b8d1ecedd5b5e81fbf1918e876752a7dd948e05c1a0dba10ab863842d45acd5
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses pcre2 libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-multibyte --with-tcsetpgrp --disable-gdbm )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
