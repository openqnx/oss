# wget 1.25.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/wget/wget-1.25.0.tar.gz |
| SHA-256 | `766e48423e79359ea31e41db9e5c289675947a7fcf2efdcedb726ac9d0da3784` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`gnutls`](../gnutls/) [`libidn2`](../libidn2/) [`zlib`](../zlib/) [`pcre2`](../pcre2/) [`libpsl`](../libpsl/) |
| Run-time dependencies | [`gnutls`](../gnutls/) [`libiconv`](../libiconv/) [`libpsl`](../libpsl/) [`libunistring`](../libunistring/) [`nettle`](../nettle/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_cli_tools`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add wget
apk add wget-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-cli-tools && /usr/local/bin/qnx-port-tests/test_cli_tools    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install wget`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild wget` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (15 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild gnutls libidn2 zlib pcre2 libpsl
```

<details><summary>Full build chain (15)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libpsl`](../libpsl/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/wget/wget-1.25.0.tar.gz
mkdir -p dist src/wget
curl -fL -o dist/$(basename $URL) $URL
echo "766e48423e79359ea31e41db9e5c289675947a7fcf2efdcedb726ac9d0da3784  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/wget --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/wget && cd build/$QNX_ARCH/wget
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/wget/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --with-ssl=gnutls \
    --disable-nls \
    --without-libuuid \
    --disable-iri \
    --without-metalink
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- wget: non-interactive HTTP/HTTPS/FTP downloader (TLS through GnuTLS, IDN, recursive mirroring).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH wget`.

### 5. What it installs

4 files under `/usr/local`, 1 programs.

```
/usr/local/bin/wget
```

The complete list is `stage/<arch>/.manifest/wget` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa wget
ls dists/8.0/*/ | grep '^wget-'
```

This writes `wget-<ver>-r0.tar.gz` (run time) and `wget-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add wget
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/wget-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/wget-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_cli_tools`](../../tests/src/) links wget the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-cli-tools`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-cli-tools
/usr/local/bin/qnx-port-tests/test_cli_tools
```

```
== wget ==
  ok   ...
PASS wget
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_cli_tools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_cli_tools`](../../tests/src/) -- a program linked against wget the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | lzip 1.26-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | patch 2.8-r0 | ✅ PASS | 2026-10-30 |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | wget 1.25.0-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r0 | ❌ FAIL | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ed 1.22.5-r1 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | lzip 1.26-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | patch 2.8-r0 | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | wget 1.25.0-r0 | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **1.25.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add wget=<version>`), sources in `ported/wget/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates wget`; port one with `scripts/port-update wget`.

<details><summary>recipe.sh</summary>

```bash
# wget: non-interactive HTTP/HTTPS/FTP downloader (TLS through GnuTLS, IDN, recursive mirroring).
VERSION=1.25.0
TARBALL=https://ftp.gnu.org/gnu/wget/wget-1.25.0.tar.gz
SHA256=766e48423e79359ea31e41db9e5c289675947a7fcf2efdcedb726ac9d0da3784
DEPS="gnutls libidn2 zlib pcre2 libpsl"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --with-ssl=gnutls --disable-nls --without-libuuid --disable-iri --without-metalink )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
