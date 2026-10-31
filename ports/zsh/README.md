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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `zsh` (programs, shared libraries, run-time data) and `zsh-dev` (headers, pkg-config/CMake files, static libraries; needs `zsh`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`zsh`](https://openqnx.github.io/oss/package/x86_64/zsh.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zsh.html)) · [`zsh-dev`](https://openqnx.github.io/oss/package/x86_64/zsh-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/zsh-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add zsh` (dependencies come along); one version: `apk add zsh=<version>` | `apk del zsh` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install zsh` (dependencies first) | `oss-get remove zsh` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`zsh-dev` the same way, by its own name (`apk add zsh-dev`, `oss-get install zsh-dev`, by hand with its own file and list `zsh-dev.list`); remove it before `zsh`, which it needs.

```sh
# on the QNX target, as root
apk add zsh zsh-dev
apk add qnx-test-zsh && /usr/local/bin/qnx-port-tests/test_zsh    # its on-target test
apk del zsh zsh-dev qnx-test-zsh
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/zsh.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/zsh.list
rm var/lib/manual/zsh.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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
