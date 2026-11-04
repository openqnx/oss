# nuspell 5.1.9

> Nuspell spell checking library and program for Hunspell dictionaries

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/nuspell/nuspell.git (tag `v5.1.9`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`icu`](../icu/) |
| Run-time dependencies | [`icu`](../icu/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_nuspell`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `nuspell` (programs, shared libraries, run-time data) and `nuspell-dev` (headers, pkg-config/CMake files, static libraries; needs `nuspell`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`nuspell`](https://openqnx.github.io/oss/package/x86_64/nuspell.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nuspell.html)) · [`nuspell-dev`](https://openqnx.github.io/oss/package/x86_64/nuspell-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/nuspell-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add nuspell` (dependencies come along); one version: `apk add nuspell=<version>` | `apk del nuspell` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install nuspell` (dependencies first) | `oss-get remove nuspell` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`nuspell-dev` the same way, by its own name (`apk add nuspell-dev`, `oss-get install nuspell-dev`, by hand with its own file and list `nuspell-dev.list`); remove it before `nuspell`, which it needs.

```sh
# on the QNX target, as root
apk add nuspell nuspell-dev
apk add qnx-test-nuspell && /usr/local/bin/qnx-port-tests/test_nuspell    # its on-target test, from tests-8.0-<arch>-m-z
apk del nuspell nuspell-dev qnx-test-nuspell
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/nuspell.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/nuspell.list
rm var/lib/manual/nuspell.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild nuspell` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild icu
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v5.1.9 --recurse-submodules --shallow-submodules \
    https://github.com/nuspell/nuspell.git src/nuspell
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/nuspell < ports/nuspell/patches/0001-strip-bom-peek.patch
```

**[`0001-strip-bom-peek.patch`](patches/0001-strip-bom-peek.patch)** — Detect the UTF-8 BOM with peek() instead of read() and putback().

> strip_utf8_bom() read the first three bytes of the .aff and .dic files and,
> when they were not a UTF-8 byte order mark, put them back with putback().
> With QNX's libc++ an istream::read() of a few bytes from a std::filebuf can
> go straight into the caller's buffer, leaving the filebuf no putback
> position: putback() then fails and sets badbit, and every dictionary
> without a BOM failed to load ("There is no number" in the .dic, "Parsing
> error"). The standard does not guarantee putback after read(), so peek at
> the first byte instead and only read the three bytes when it is the BOM's
> first byte (0xEF). Files with a BOM behave as before.

Touches: `src/nuspell/aff_data.cxx` · Upstream status: Pending (portable: putback after read() is not guaranteed by the standard)

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/nuspell -B build/$QNX_ARCH/nuspell -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TESTING=OFF \
      -DBUILD_DOCS=OFF
cmake --build build/$QNX_ARCH/nuspell -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/nuspell
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Nuspell: a spell checking library and program that reads Hunspell dictionaries (.aff/.dic), with Unicode handling through ICU (libnuspell, the nuspell program). QNX's package list has 5.1.6; this is the latest release of the 5.1 series.
- No tests (they need Catch2) and no man pages (they need pandoc).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH nuspell`.

### 5. What it installs

19 files under `/usr/local`: 3 shared-library entries, 1 programs, 9 headers.

```
/usr/local/lib/libnuspell.so
/usr/local/lib/libnuspell.so.5
/usr/local/lib/libnuspell.so.5.1.9
/usr/local/bin/nuspell
```

The complete list is `stage/<arch>/.manifest/nuspell` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa nuspell
ls dists/8.0/*/ | grep '^nuspell-'
```

This writes `nuspell-<ver>-r0.tar.gz` (run time) and `nuspell-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add nuspell
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/nuspell-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/nuspell-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_nuspell`](../../tests/src/) links nuspell the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-nuspell`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-nuspell
/usr/local/bin/qnx-port-tests/test_nuspell
```

```
== nuspell ==
  ok   ...
PASS nuspell
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_nuspell` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_nuspell`](../../tests/src/) -- a program linked against nuspell the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - load_aff_dic(path)
   - load_aff_dic(istream, istream): waited / not waitted
   - a dictionary with a UTF-8 BOM loads: cats, caf\xc3\xa9 / not catz

Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> nuspell`.

## Versions

This recipe builds **5.1.9**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add nuspell=<version>`), sources in `ported/nuspell/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates nuspell`; port one with `scripts/port-update nuspell`.

<details><summary>recipe.sh</summary>

```bash
# Nuspell: a spell checking library and program that reads Hunspell
# dictionaries (.aff/.dic), with Unicode handling through ICU
# (libnuspell, the nuspell program). QNX's package list has 5.1.6; this is
# the latest release of the 5.1 series.
VERSION=5.1.9; GIT=https://github.com/nuspell/nuspell.git; REF=v5.1.9
DEPS="icu"
# No tests (they need Catch2) and no man pages (they need pandoc).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DBUILD_TESTING=OFF -DBUILD_DOCS=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
