# stringencoders 3.10.3-git-20180306

> Fast C string encoders and decoders (base64, base85, URL, JSON, ...)

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/client9/stringencoders.git |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_stringencoders`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `stringencoders` (programs, shared libraries, run-time data) and `stringencoders-dev` (headers, pkg-config/CMake files, static libraries; needs `stringencoders`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`stringencoders`](https://openqnx.github.io/oss/package/x86_64/stringencoders.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/stringencoders.html)) · [`stringencoders-dev`](https://openqnx.github.io/oss/package/x86_64/stringencoders-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/stringencoders-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add stringencoders` (dependencies come along); one version: `apk add stringencoders=<version>` | `apk del stringencoders` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install stringencoders` (dependencies first) | `oss-get remove stringencoders` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`stringencoders-dev` the same way, by its own name (`apk add stringencoders-dev`, `oss-get install stringencoders-dev`, by hand with its own file and list `stringencoders-dev.list`); remove it before `stringencoders`, which it needs.

```sh
# on the QNX target, as root
apk add stringencoders stringencoders-dev
apk add qnx-test-stringencoders && /usr/local/bin/qnx-port-tests/test_stringencoders    # its on-target test, from tests-8.0-<arch>-m-z
apk del stringencoders stringencoders-dev qnx-test-stringencoders
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/stringencoders.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/stringencoders.list
rm var/lib/manual/stringencoders.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild stringencoders` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/client9/stringencoders.git src/stringencoders
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/stringencoders < ports/stringencoders/patches/0001-generators-for-build-machine.patch
patch -p1 -d src/stringencoders < ports/stringencoders/patches/0002-install-modp-stdint-h.patch
```

**[`0001-generators-for-build-machine.patch`](patches/0001-generators-for-build-machine.patch)** — Build the table generators for the build machine (cross compiling)

> The base2/16/64/85, URL, JavaScript and ASCII lookup tables
> (modp_*_data.h) are written during the build by small generator programs.
> Automake built them as noinst_PROGRAMS with the target compiler, so in a
> cross build for QNX the build then tried to run QNX executables on the
> build machine. They are now built with CC_FOR_BUILD (the build machine's
> cc when cross compiling, $CC otherwise), which configure takes as a
> precious variable. The two perl rules that derive modp_b64w.c and
> modp_b64r.c read modp_b64.c from the current directory, which fails in a
> build directory separate from the source; they read it from $(srcdir) (and
> drop perl's -i, meaningless on a pipe).

Touches: `configure.ac`, `src/Makefile.am` · Upstream status: Inappropriate (upstream archived)

**[`0002-install-modp-stdint-h.patch`](patches/0002-install-modp-stdint-h.patch)** — Install modp_stdint.h with the other headers

> modp_b64.h, modp_b16.h and the other installed headers include
> "modp_stdint.h", which upstream does not install: no installed header could
> be compiled. It is installed with them.

Touches: `src/Makefile.am` · Upstream status: Inappropriate (upstream archived)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/stringencoders; B=build/$QNX_ARCH/stringencoders; mkdir -p $B
```

```bash
do_build() { make -C "$B/src" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B/src" DESTDIR="$STAGE" install; }
```

```bash
do_patch() { touch "$SRCROOT/src/modp_json_data.h"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Nick Galbreath's stringencoders (libmodpbase64): fast base64/base85/ base16/base2, URL, JavaScript, JSON, XML/HTML and number encoders and decoders in C. The project is archived; the last release tag (v3.10.3) predates fixes on master, so this is the final commit.
- The lookup tables are written at build time by small generator programs, which a cross build compiles for QNX and then cannot run: patches/0001 builds them with the build machine's compiler (CC_FOR_BUILD) and makes the rules work outside the source directory. patches/0002 installs modp_stdint.h, which the installed headers include. Only src/ is built: test/ holds the upstream unit tests, which also run at build time.
- modp_json_data.h is committed, generated by a Python script; keep the checked-out copy newer than the script so the build never reruns it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH stringencoders`.

### 5. What it installs

20 files under `/usr/local`: 2 shared-library entries, 17 headers.

```
/usr/local/lib/libmodpbase64.so
/usr/local/lib/libmodpbase64.so.0
```

The complete list is `stage/<arch>/.manifest/stringencoders` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa stringencoders
ls dists/8.0/*/ | grep '^stringencoders-'
```

This writes `stringencoders-<ver>-r0.tar.gz` (run time) and `stringencoders-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add stringencoders
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/stringencoders-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/stringencoders-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_stringencoders`](../../tests/src/) links stringencoders the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-stringencoders`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-stringencoders
/usr/local/bin/qnx-port-tests/test_stringencoders
```

```
== stringencoders ==
  ok   ...
PASS stringencoders
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_stringencoders` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_stringencoders`](../../tests/src/) -- a program linked against stringencoders the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - base64 encode/decode: the 7 RFC 4648 test vectors
   - base64 decode of an invalid character returns -1
   - base64 round trip of bytes 0..255
   - base85 round trip of bytes 0..255
   - base16 encode: 514E5801FF (upper case, as upstream documents)
   - base16 decode (upper case)
   - URL encode: a+b%26c%3Dd%2F%E9
   - URL decode: + and %xx
   - JavaScript string escaping
   - modp_itoa10(INT32_MIN)
   - modp_ulitoa10(UINT64_MAX)
   - modp_dtoa(3.14159, 3) = 3.142
   - modp_dtoa2(-0.5, 6) = -0.5
   - modp_toupper

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | stringencoders 3.10.3_git20180306-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/stringencoders-3.10.3_git20180306-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | stringencoders 3.10.3_git20180306-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/stringencoders-3.10.3_git20180306-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.10.3-git-20180306**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add stringencoders=<version>`), sources in `ported/stringencoders/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates stringencoders`; port one with `scripts/port-update stringencoders`.

<details><summary>recipe.sh</summary>

```bash
# Nick Galbreath's stringencoders (libmodpbase64): fast base64/base85/
# base16/base2, URL, JavaScript, JSON, XML/HTML and number encoders and
# decoders in C. The project is archived; the last release tag (v3.10.3)
# predates fixes on master, so this is the final commit.
VERSION=3.10.3-git-20180306; GIT=https://github.com/client9/stringencoders.git; COMMIT=e1448a9415f4ebf6f559c86718193ba067cbb99d
BUILDSYS=autotools
# The lookup tables are written at build time by small generator programs,
# which a cross build compiles for QNX and then cannot run: patches/0001
# builds them with the build machine's compiler (CC_FOR_BUILD) and makes the
# rules work outside the source directory. patches/0002 installs
# modp_stdint.h, which the installed headers include. Only src/ is built:
# test/ holds the upstream unit tests, which also run at build time.
CONF_ENV=( CC_FOR_BUILD=gcc )
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
do_build() { make -C "$B/src" -j"$(nproc)"; }
do_install() { make -C "$B/src" DESTDIR="$STAGE" install; }
# modp_json_data.h is committed, generated by a Python script; keep the
# checked-out copy newer than the script so the build never reruns it.
do_patch() { touch "$SRCROOT/src/modp_json_data.h"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
