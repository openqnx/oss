# itstool 2.0.7

> Translates XML documents with PO files using W3C ITS rules

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2 |
| SHA-256 | `6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`python3`](../python3/) [`libxml2`](../libxml2/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_itstool`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add itstool
apk add itstool-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-itstool && /usr/local/bin/qnx-port-tests/test_itstool    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install itstool`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild itstool` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild python3 libxml2
```

<details><summary>Full build chain (16)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2
mkdir -p dist src/itstool
curl -fL -o dist/$(basename $URL) $URL
echo "6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/itstool --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/itstool < ports/itstool/patches/0001-skip-libxml2-module-check-when-cross.patch
```

**[`0001-skip-libxml2-module-check-when-cross.patch`](patches/0001-skip-libxml2-module-check-when-cross.patch)** — Skip the libxml2 Python module check when cross compiling.

> configure imports libxml2 with the build machine's Python, which says
> nothing about the target: itstool needs the module in the Python it runs
> under on the target. Skip the test in a cross build ("maybe" included:
> itstool checks no compiler, so configure never settles it to "yes").

Touches: `configure.ac` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/itstool && cd build/$QNX_ARCH/itstool
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/itstool/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- itstool (XML translation tool; Python). Version as in the QNX Everywhere 8.0.4r1 image.
- The QNX patch changes configure.ac; regenerate configure from it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH itstool`.

### 5. What it installs

8 files under `/usr/local`, 1 programs.

```
/usr/local/bin/itstool
```

The complete list is `stage/<arch>/.manifest/itstool` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa itstool
ls dists/8.0/*/ | grep '^itstool-'
```

This writes `itstool-<ver>-r0.tar.gz` (run time) and `itstool-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add itstool
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/itstool-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/itstool-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_itstool`](../../tests/src/) links itstool the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-itstool`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-itstool
/usr/local/bin/qnx-port-tests/test_itstool
```

```
== itstool ==
  ok   ...
PASS itstool
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_itstool` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_itstool`](../../tests/src/) -- a program linked against itstool the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | itstool 2.0.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/itstool-2.0.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | itstool 2.0.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/itstool-2.0.7-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.0.7**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add itstool=<version>`), sources in `ported/itstool/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates itstool`; port one with `scripts/port-update itstool`.

<details><summary>recipe.sh</summary>

```bash
# itstool (XML translation tool; Python). Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.0.7
TARBALL=https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2
SHA256=6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca
DEPS="python3 libxml2"
BUILDSYS=autotools
CONF_ARGS_EXTRA=(  )
# The QNX patch changes configure.ac; regenerate configure from it.
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
