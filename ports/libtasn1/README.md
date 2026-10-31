# libtasn1 4.19.0

> GNU library for ASN.1 and DER manipulation

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/libtasn1/libtasn1-4.19.0.tar.gz |
| SHA-256 | `1613f0ac1cf484d6ec0ce3b8c06d56263cc7242f1c23b30d82d23de345a63f7a` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libtasn1`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libtasn1
apk add libtasn1-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libtasn1 && /usr/local/bin/qnx-port-tests/test_libtasn1    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libtasn1`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libtasn1` performs exactly the same steps (2–5) in one command.

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
URL=https://ftp.gnu.org/gnu/libtasn1/libtasn1-4.19.0.tar.gz
mkdir -p dist src/libtasn1
curl -fL -o dist/$(basename $URL) $URL
echo "1613f0ac1cf484d6ec0ce3b8c06d56263cc7242f1c23b30d82d23de345a63f7a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libtasn1 --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libtasn1 && cd build/$QNX_ARCH/libtasn1
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libtasn1/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-doc
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libtasn1`.

### 5. What it installs

7 files under `/usr/local`: 2 shared-library entries, 3 programs, 1 headers.

```
/usr/local/lib/libtasn1.so
/usr/local/lib/libtasn1.so.12
/usr/local/bin/asn1Coding
/usr/local/bin/asn1Decoding
/usr/local/bin/asn1Parser
```

The complete list is `stage/<arch>/.manifest/libtasn1` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libtasn1
ls dists/8.0/*/ | grep '^libtasn1-'
```

This writes `libtasn1-<ver>-r0.tar.gz` (run time) and `libtasn1-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libtasn1
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libtasn1-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libtasn1-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libtasn1`](../../tests/src/) links libtasn1 the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libtasn1`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libtasn1
/usr/local/bin/qnx-port-tests/test_libtasn1
```

```
== libtasn1 ==
  ok   ...
PASS libtasn1
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libtasn1` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libtasn1`](../../tests/src/) -- a program linked against libtasn1 the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - asn1_encode_simple_der
   - tag/length octets are 04 02
   - full TLV is 04 02 12 34
   - asn1_decode_simple_der
   - decoded value
   - long-form length for 300
   - asn1_get_length_der round trip
   - asn1_strerror

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libtasn1 4.19.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libtasn1-4.19.0-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libtasn1 4.19.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libtasn1-4.19.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.19.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libtasn1=<version>`), sources in `ported/libtasn1/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libtasn1`; port one with `scripts/port-update libtasn1`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.19.0
TARBALL=https://ftp.gnu.org/gnu/libtasn1/libtasn1-4.19.0.tar.gz
SHA256=1613f0ac1cf484d6ec0ce3b8c06d56263cc7242f1c23b30d82d23de345a63f7a
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-doc )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
