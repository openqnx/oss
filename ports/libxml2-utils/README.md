# libxml2-utils 2.13.5

> xmllint and xmlcatalog, the command-line tools of libxml2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/libxml2.git (tag `v2.13.5`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`libxml2`](../libxml2/) [`zlib`](../zlib/) [`xz`](../xz/) |
| Run-time dependencies | [`libxml2`](../libxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxml2_utils`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `libxml2-utils`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libxml2-utils`](https://openqnx.github.io/oss/package/x86_64/libxml2-utils.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libxml2-utils.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libxml2-utils` (dependencies come along); one version: `apk add libxml2-utils=<version>` | `apk del libxml2-utils` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libxml2-utils` (dependencies first) | `oss-get remove libxml2-utils` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-libxml2-utils && /usr/local/bin/qnx-port-tests/test_libxml2_utils    # its on-target test, from tests-8.0-<arch>
apk del libxml2-utils qnx-test-libxml2-utils
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libxml2-utils.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libxml2-utils.list
rm var/lib/manual/libxml2-utils.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxml2-utils` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libxml2 zlib xz
```

<details><summary>Full build chain (16)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v2.13.5 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/libxml2.git src/libxml2-utils
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/libxml2-utils; B=build/$QNX_ARCH/libxml2-utils; mkdir -p $B
```

```bash
do_build() { cmake --build "$B" -j"$(nproc)" --target xmllint xmlcatalog; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/xmllint" "$B/xmlcatalog" "$STAGE$PREFIX/bin/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xmllint and xmlcatalog, libxml2's command-line tools: parse, validate (DTD, RelaxNG, XML Schema), pretty-print and query XML, and maintain XML catalogs. The libxml2 component is built without its programs; this builds them from the same release and installs only the programs, which link that component's libxml2.so.2.
- Same options as the libxml2 component except the Python bindings (not needed for the programs). No readline (xmllint --shell then reads plain lines): libxml2 2.13's CMakeLists links the programs with libhistory whenever it finds one, whatever LIBXML2_WITH_HISTORY says, so the probe results are preset. The programs are installed by hand, so they are linked without the build tree's RPATH.
- Their man pages are already in the libxml2 component.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxml2-utils`.

### 5. What it installs

2 files under `/usr/local`, 2 programs.

```
/usr/local/bin/xmlcatalog
/usr/local/bin/xmllint
```

The complete list is `stage/<arch>/.manifest/libxml2-utils` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxml2-utils
ls dists/8.0/*/ | grep '^libxml2-utils-'
```

This writes `libxml2-utils-<ver>-r0.tar.gz` (run time) and `libxml2-utils-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxml2-utils
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxml2-utils-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxml2-utils-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxml2_utils`](../../tests/src/) links libxml2-utils the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxml2-utils`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxml2-utils
/usr/local/bin/qnx-port-tests/test_libxml2_utils
```

```
== libxml2-utils ==
  ok   ...
PASS libxml2-utils
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxml2_utils` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxml2_utils`](../../tests/src/) -- a program linked against libxml2-utils the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxml2-utils 2.13.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libxml2-utils-2.13.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxml2-utils 2.13.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libxml2-utils-2.13.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.13.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxml2-utils=<version>`), sources in `ported/libxml2-utils/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxml2-utils`; port one with `scripts/port-update libxml2-utils`.

<details><summary>recipe.sh</summary>

```bash
# xmllint and xmlcatalog, libxml2's command-line tools: parse, validate
# (DTD, RelaxNG, XML Schema), pretty-print and query XML, and maintain XML
# catalogs. The libxml2 component is built without its programs; this
# builds them from the same release and installs only the programs, which
# link that component's libxml2.so.2.
VERSION=2.13.5; GIT=https://gitlab.gnome.org/GNOME/libxml2.git; REF=v2.13.5
DEPS="libxml2 zlib xz"
# Same options as the libxml2 component except the Python bindings (not
# needed for the programs). No readline (xmllint --shell then reads plain
# lines): libxml2 2.13's CMakeLists links the programs with libhistory
# whenever it finds one, whatever LIBXML2_WITH_HISTORY says, so the probe
# results are preset. The programs are installed by hand, so they are
# linked without the build tree's RPATH.
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DLIBXML2_WITH_PYTHON=OFF -DLIBXML2_WITH_TESTS=OFF
                   -DLIBXML2_WITH_PROGRAMS=ON -DLIBXML2_WITH_ICONV=OFF -DLIBXML2_WITH_LZMA=ON
                   -DLIBXML2_WITH_ZLIB=ON -DLIBXML2_WITH_READLINE=OFF -DLIBXML2_WITH_HISTORY=OFF
                   -DHAVE_LIBHISTORY=0 -DHAVE_LIBREADLINE=0 -DCMAKE_SKIP_BUILD_RPATH=ON
                   -DCMAKE_POSITION_INDEPENDENT_CODE=ON )
do_build() { cmake --build "$B" -j"$(nproc)" --target xmllint xmlcatalog; }
# Their man pages are already in the libxml2 component.
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/xmllint" "$B/xmlcatalog" "$STAGE$PREFIX/bin/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
