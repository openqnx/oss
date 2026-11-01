# jq 1.7.1

> Command line JSON processor and libjq library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-1.7.1.tar.gz |
| SHA-256 | `478c9ca129fd2e3443fe27314b455e211e0d8c60bc8ff7df703873deeee580c2` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`oniguruma`](../oniguruma/) |
| Run-time dependencies | [`oniguruma`](../oniguruma/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_jq`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `jq` (programs, shared libraries, run-time data) and `jq-dev` (headers, pkg-config/CMake files, static libraries; needs `jq`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`jq`](https://openqnx.github.io/oss/package/x86_64/jq.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/jq.html)) · [`jq-dev`](https://openqnx.github.io/oss/package/x86_64/jq-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/jq-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add jq` (dependencies come along); one version: `apk add jq=<version>` | `apk del jq` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install jq` (dependencies first) | `oss-get remove jq` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`jq-dev` the same way, by its own name (`apk add jq-dev`, `oss-get install jq-dev`, by hand with its own file and list `jq-dev.list`); remove it before `jq`, which it needs.

```sh
# on the QNX target, as root
apk add jq jq-dev
apk add qnx-test-jq && /usr/local/bin/qnx-port-tests/test_jq    # its on-target test, from tests-8.0-<arch>
apk del jq jq-dev qnx-test-jq
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/jq.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/jq.list
rm var/lib/manual/jq.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild jq` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild oniguruma
```

### 2. Get the upstream source

```bash
URL=https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-1.7.1.tar.gz
mkdir -p dist src/jq
curl -fL -o dist/$(basename $URL) $URL
echo "478c9ca129fd2e3443fe27314b455e211e0d8c60bc8ff7df703873deeee580c2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/jq --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/jq < ports/jq/patches/0001-qnx-xopen-source-value.patch
```

**[`0001-qnx-xopen-source-value.patch`](patches/0001-qnx-xopen-source-value.patch)** — jq: give _XOPEN_SOURCE a value on QNX.

> QNX's <sys/platform.h> accepts only 500, 600 or 700; the valueless definition
> in builtin.c stops with "This version of XOPEN_SOURCE is not supported".

Touches: `src/builtin.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/jq && cd build/$QNX_ARCH/jq
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/jq/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-docs \
    --disable-maintainer-mode \
    --with-oniguruma="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag: jq's lexer.c and parser.c are generated by flex and bison, and the generator only runs under maintainer mode, which this build disables. A git checkout therefore has neither file and the build dies on "src/lexer.c: No such file or directory" -- while printing the reason a few lines earlier, "NOT building lexer.c!". The tarball ships both. This never surfaced on the machine the port was developed on, because an earlier build there had left the generated files behind in src/jq.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH jq`.

### 5. What it installs

11 files under `/usr/local`: 2 shared-library entries, 1 programs, 2 headers.

```
/usr/local/lib/libjq.so
/usr/local/lib/libjq.so.1
/usr/local/bin/jq
```

The complete list is `stage/<arch>/.manifest/jq` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa jq
ls dists/8.0/*/ | grep '^jq-'
```

This writes `jq-<ver>-r0.tar.gz` (run time) and `jq-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add jq
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/jq-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/jq-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_jq`](../../tests/src/) links jq the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-jq`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-jq
/usr/local/bin/qnx-port-tests/test_jq
```

```
== jq ==
  ok   ...
PASS jq
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_jq` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_jq`](../../tests/src/) -- a program linked against jq the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - jq_init
   - jq_compile
   - jv_parse
   - jq_next produced a result
   - the program returned 2
   - only one result
   - compile a program using built-ins
   - result is a string
   - built-ins produced the expected string
   - a bad program fails to compile

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64 virt, vanilla mkqnximage image | apk:local-dists | jq 1.7.1-r0 | ✅ PASS | 2026-10-30 | [log](../../tests/logs/aarch64le/jq-1.7.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | jq 1.7.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/jq-1.7.1-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | jq 1.7.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/jq-1.7.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.7.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add jq=<version>`), sources in `ported/jq/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates jq`; port one with `scripts/port-update jq`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.7.1
# The release tarball, not the git tag: jq's lexer.c and parser.c are generated
# by flex and bison, and the generator only runs under maintainer mode, which
# this build disables. A git checkout therefore has neither file and the build
# dies on "src/lexer.c: No such file or directory" -- while printing the reason
# a few lines earlier, "NOT building lexer.c!". The tarball ships both.
#
# This never surfaced on the machine the port was developed on, because an
# earlier build there had left the generated files behind in src/jq.
TARBALL=https://github.com/jqlang/jq/releases/download/jq-1.7.1/jq-1.7.1.tar.gz
SHA256=478c9ca129fd2e3443fe27314b455e211e0d8c60bc8ff7df703873deeee580c2
DEPS="oniguruma"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-docs --disable-maintainer-mode
                  --with-oniguruma="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
