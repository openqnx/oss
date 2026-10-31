# bash 5.2.37

> GNU Bourne Again shell

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/bash/bash-5.2.37.tar.gz |
| SHA-256 | `9599b22ecd1d5787ad7d3b7bf0c59f312b3396d1e281175dd1f8a4014da621ff` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`ncurses`](../ncurses/) [`readline`](../readline/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`ncurses`](../ncurses/) [`readline`](../readline/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_bash`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add bash
apk add bash-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-bash && /usr/local/bin/qnx-port-tests/test_bash    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install bash`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild bash` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses readline libiconv
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/bash/bash-5.2.37.tar.gz
mkdir -p dist src/bash
curl -fL -o dist/$(basename $URL) $URL
echo "9599b22ecd1d5787ad7d3b7bf0c59f312b3396d1e281175dd1f8a4014da621ff  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/bash --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/bash < ports/bash/patches/0001-qnx8-drop-qnx4-watcom-flags.patch
```

**[`0001-qnx8-drop-qnx4-watcom-flags.patch`](patches/0001-qnx8-drop-qnx4-watcom-flags.patch)** — Do not apply the QNX 4 (Watcom) compiler flags to QNX Neutrino.

> bash's configure matches every *qnx* host and adds "-Dqnx -F -3s" and
> -lunix: Watcom options from QNX 4. Neutrino (nto-qnx8.0.0) is GCC-based; both
> the cross compiler and the build-host gcc that compiles mksyntax reject -3s.
> Restrict the case to the old qnx4 host triplets.

Touches: `configure.ac`, `configure` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/bash && cd build/$QNX_ARCH/bash
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/bash/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --without-bash-malloc \
    --disable-nls \
    --with-installed-readline \
    --with-curses
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libiconv

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH bash`.

### 5. What it installs

15 files under `/usr/local`, 2 programs.

```
/usr/local/bin/bash
/usr/local/bin/bashbug
```

The complete list is `stage/<arch>/.manifest/bash` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa bash
ls dists/8.0/*/ | grep '^bash-'
```

This writes `bash-<ver>-r0.tar.gz` (run time) and `bash-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add bash
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/bash-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/bash-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_bash`](../../tests/src/) links bash the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-bash`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-bash
/usr/local/bin/qnx-port-tests/test_bash
```

```
== bash ==
  ok   ...
PASS bash
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_bash` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_bash`](../../tests/src/) -- a program linked against bash the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | bash 5.2.37-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/bash-5.2.37-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | bash 5.2.37-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/bash-5.2.37-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.2.37**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add bash=<version>`), sources in `ported/bash/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates bash`; port one with `scripts/port-update bash`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.2.37
TARBALL=https://ftp.gnu.org/gnu/bash/bash-5.2.37.tar.gz
SHA256=9599b22ecd1d5787ad7d3b7bf0c59f312b3396d1e281175dd1f8a4014da621ff
# Also linked at run time (found by scripts/gen-dependency-index): libiconv
DEPS="ncurses readline libiconv"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --without-bash-malloc --disable-nls --with-installed-readline --with-curses )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
