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

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `itstool` (programs, shared libraries, run-time data) and `itstool-dev` (headers, pkg-config/CMake files, static libraries; needs `itstool`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`itstool`](https://openqnx.github.io/oss/package/x86_64/itstool.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/itstool.html)) · [`itstool-dev`](https://openqnx.github.io/oss/package/x86_64/itstool-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/itstool-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add itstool` (dependencies come along); one version: `apk add itstool=<version>` | `apk del itstool` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install itstool` (dependencies first) | `oss-get remove itstool` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`itstool-dev` the same way, by its own name (`apk add itstool-dev`, `oss-get install itstool-dev`, by hand with its own file and list `itstool-dev.list`); remove it before `itstool`, which it needs.

```sh
# on the QNX target, as root
apk add itstool itstool-dev
apk add qnx-test-itstool && /usr/local/bin/qnx-port-tests/test_itstool    # its on-target test
apk del itstool itstool-dev qnx-test-itstool
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/itstool.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/itstool.list
rm var/lib/manual/itstool.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

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

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/itstool; B=build/$QNX_ARCH/itstool; mkdir -p $B
```

```bash
do_patch() { cd "$SRCROOT" && autoreconf -fi; }
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i "1s|^#!.*|#!$PREFIX/bin/python3 -s|" "$STAGE$PREFIX/bin/itstool"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- itstool (XML translation tool; Python). Version as in the QNX Everywhere 8.0.4r1 image.
- The QNX patch changes configure.ac; regenerate configure from it.
- itstool is a Python script that imports libxml2's bindings (libxml2.py, libxml2mod.so in site-packages): no DT_NEEDED says so.
- itstool's #! line is the build machine's python3 (configure's PYTHON); the target's python3 is $PREFIX/bin/python3 (hazard 105).

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
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | itstool 2.0.7-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/itstool-2.0.7-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | itstool 2.0.7-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/itstool-2.0.7-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | itstool 2.0.7-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/itstool-2.0.7-r2.log) |

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

# itstool is a Python script that imports libxml2's bindings (libxml2.py,
# libxml2mod.so in site-packages): no DT_NEEDED says so.
RUNTIME_DEPS="python3 libxml2"
# itstool's #! line is the build machine's python3 (configure's PYTHON);
# the target's python3 is $PREFIX/bin/python3 (hazard 105).
do_install() {
  make -C "$B" DESTDIR="$STAGE" install
  sed -i "1s|^#!.*|#!$PREFIX/bin/python3 -s|" "$STAGE$PREFIX/bin/itstool"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
