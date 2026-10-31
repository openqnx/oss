# libgpg-error 1.50

> Common error values and runtime library for GnuPG components

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2 |
| SHA-256 | `69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`gettext`](../gettext/) |
| Run-time dependencies | [`gettext`](../gettext/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libgpg_error`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `libgpg-error` (programs, shared libraries, run-time data) and `libgpg-error-dev` (headers, pkg-config/CMake files, static libraries; needs `libgpg-error`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`libgpg-error`](https://openqnx.github.io/oss/package/x86_64/libgpg-error.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libgpg-error.html)) · [`libgpg-error-dev`](https://openqnx.github.io/oss/package/x86_64/libgpg-error-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/libgpg-error-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add libgpg-error` (dependencies come along); one version: `apk add libgpg-error=<version>` | `apk del libgpg-error` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install libgpg-error` (dependencies first) | `oss-get remove libgpg-error` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`libgpg-error-dev` the same way, by its own name (`apk add libgpg-error-dev`, `oss-get install libgpg-error-dev`, by hand with its own file and list `libgpg-error-dev.list`); remove it before `libgpg-error`, which it needs.

```sh
# on the QNX target, as root
apk add libgpg-error libgpg-error-dev
apk add qnx-test-libgpg-error && /usr/local/bin/qnx-port-tests/test_libgpg_error    # its on-target test
apk del libgpg-error libgpg-error-dev qnx-test-libgpg-error
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/libgpg-error.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/libgpg-error.list
rm var/lib/manual/libgpg-error.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libgpg-error` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gettext
```

<details><summary>Full build chain (4)</summary>

[`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
URL=https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2
mkdir -p dist src/libgpg-error
curl -fL -o dist/$(basename $URL) $URL
echo "69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libgpg-error --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libgpg-error && cd build/$QNX_ARCH/libgpg-error
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libgpg-error/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-tests \
    --disable-languages
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): gettext
- libgpg-error needs a syscfg/lock-obj-pub.<triplet>.h describing the target's pthread_mutex_t layout, and ships none for QNX. The two under files/ were produced by building the upstream generator (src/gen-posix-lock-obj.c) for QNX and running it on the real targets -- a Raspberry Pi 5 for aarch64le and the QEMU VM for x86_64. QNX's pthread_mutex_t is 8 bytes on both and the two generated headers are byte-identical, so a CPU-less lock-obj-pub.nto-qnx.h is shipped as well: mkheader falls back to that spelling for the x86_64 triplet.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libgpg-error`.

### 5. What it installs

30 files under `/usr/local`: 2 shared-library entries, 2 programs, 2 headers.

```
/usr/local/lib/libgpg-error.so
/usr/local/lib/libgpg-error.so.37
/usr/local/bin/gpg-error
/usr/local/bin/gpgrt-config
```

The complete list is `stage/<arch>/.manifest/libgpg-error` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libgpg-error
ls dists/8.0/*/ | grep '^libgpg-error-'
```

This writes `libgpg-error-<ver>-r0.tar.gz` (run time) and `libgpg-error-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libgpg-error
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libgpg-error-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libgpg-error-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libgpg_error`](../../tests/src/) links libgpg-error the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libgpg-error`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libgpg-error
/usr/local/bin/qnx-port-tests/test_libgpg_error
```

```
== libgpg-error ==
  ok   ...
PASS libgpg-error
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libgpg_error` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libgpg_error`](../../tests/src/) -- a program linked against libgpg-error the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - library version 1.50
   - check_version(\"1.40\") satisfied
   - check_version(\"9.0\") not satisfied
   - strerror(GPG_ERR_GENERAL) == \"General error\"
   - strerror(GPG_ERR_BAD_SIGNATURE)
   - strerror(0) == \"Success\"
   - gpg_err_make: code and source round trip
   - strsource(GCRYPT) == \"gcrypt\"
   - strerror_r(GPG_ERR_NO_DATA)
   - errno ENOENT -> GPG_ERR_ENOENT
   - GPG_ERR_ENOENT -> errno ENOENT
   - errno EINVAL -> GPG_ERR_EINVAL
   - gpgrt_fopenmem
   - gpgrt_asprintf
   - gpgrt_strconcat
   - gpgrt_fnameconcat
   - gpgrt_b64enc_start
   - b64enc write/finish
   - base64(\"hello QNX\") == aGVsbG8gUU5Y
   - gpg-error 8 -> Bad signature
   - gpg-error decodes source and code of a combined value
   - gpg-error by symbol name

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libgpg-error 1.50-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libgpg-error-1.50-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libgpg-error 1.50-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/libgpg-error-1.50-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libgpg-error 1.50-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libgpg-error-1.50-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libgpg-error 1.50-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/libgpg-error-1.50-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.50**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libgpg-error=<version>`), sources in `ported/libgpg-error/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libgpg-error`; port one with `scripts/port-update libgpg-error`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.50
# Also linked at run time (found by scripts/gen-dependency-index): gettext
DEPS="gettext"
TARBALL=https://gnupg.org/ftp/gcrypt/libgpg-error/libgpg-error-1.50.tar.bz2
SHA256=69405349e0a633e444a28c5b35ce8f14484684518a508dc48a089992fe93e20a
BUILDSYS=autotools
# libgpg-error needs a syscfg/lock-obj-pub.<triplet>.h describing the target's
# pthread_mutex_t layout, and ships none for QNX. The two under files/ were
# produced by building the upstream generator (src/gen-posix-lock-obj.c) for QNX
# and running it on the real targets -- a Raspberry Pi 5 for aarch64le and the
# QEMU VM for x86_64. QNX's pthread_mutex_t is 8 bytes on both and the two
# generated headers are byte-identical, so a CPU-less lock-obj-pub.nto-qnx.h is
# shipped as well: mkheader falls back to that spelling for the x86_64 triplet.
do_patch() {
  cp "$RECIPE_DIR/libgpg-error/files/lock-obj-pub."*.h "$SRCROOT/src/syscfg/"
  ls "$SRCROOT/src/syscfg/" | grep nto-qnx
}

CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --disable-languages )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
