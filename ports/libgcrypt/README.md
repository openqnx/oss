# libgcrypt 1.11.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.11.0.tar.bz2 |
| SHA-256 | `09120c9867ce7f2081d6aaa1775386b98c2f2f246135761aae47d81f58685b9c` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libgpg-error`](../libgpg-error/) |
| Run-time dependencies | [`libgpg-error`](../libgpg-error/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libgcrypt`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libgcrypt
apk add libgcrypt-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libgcrypt && /usr/local/bin/qnx-port-tests/test_libgcrypt    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libgcrypt`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libgcrypt` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libgpg-error
```

<details><summary>Full build chain (5)</summary>

[`gettext`](../gettext/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
URL=https://gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.11.0.tar.bz2
mkdir -p dist src/libgcrypt
curl -fL -o dist/$(basename $URL) $URL
echo "09120c9867ce7f2081d6aaa1775386b98c2f2f246135761aae47d81f58685b9c  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libgcrypt --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libgcrypt && cd build/$QNX_ARCH/libgcrypt
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" \
$OLDPWD/src/libgcrypt/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc \
    --disable-tests \
    --with-libgpg-error-prefix="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- configure probes PATH for gpgrt-config and would otherwise pick up the build host's copy, which reports the host's libgpg-error rather than ours.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libgcrypt`.

### 5. What it installs

9 files under `/usr/local`: 2 shared-library entries, 4 programs, 1 headers.

```
/usr/local/lib/libgcrypt.so
/usr/local/lib/libgcrypt.so.25
/usr/local/bin/dumpsexp
/usr/local/bin/hmac256
/usr/local/bin/libgcrypt-config
/usr/local/bin/mpicalc
```

The complete list is `stage/<arch>/.manifest/libgcrypt` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libgcrypt
ls dists/8.0/*/ | grep '^libgcrypt-'
```

This writes `libgcrypt-<ver>-r0.tar.gz` (run time) and `libgcrypt-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libgcrypt
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libgcrypt-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libgcrypt-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libgcrypt`](../../tests/src/) links libgcrypt the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libgcrypt`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libgcrypt
/usr/local/bin/qnx-port-tests/test_libgcrypt
```

```
== libgcrypt ==
  ok   ...
PASS libgcrypt
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libgcrypt` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libgcrypt`](../../tests/src/) -- a program linked against libgcrypt the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - gcry_check_version matches the headers
   - initialisation finished
   - SHA-256(\"abc\") matches FIPS 180-2
   - gcry_cipher_open AES-128-CBC
   - encrypt
   - decrypt
   - AES-128-CBC round trip
   - two random draws differ

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libgcrypt 1.11.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libgcrypt 1.11.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.11.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libgcrypt=<version>`), sources in `ported/libgcrypt/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libgcrypt`; port one with `scripts/port-update libgcrypt`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.11.0
TARBALL=https://gnupg.org/ftp/gcrypt/libgcrypt/libgcrypt-1.11.0.tar.bz2
SHA256=09120c9867ce7f2081d6aaa1775386b98c2f2f246135761aae47d81f58685b9c
BUILDSYS=autotools
DEPS="libgpg-error"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc --disable-tests
                  --with-libgpg-error-prefix="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
