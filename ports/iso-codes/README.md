# iso-codes 4.17.0

> Lists of ISO country, language, script and currency codes with translations

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://salsa.debian.org/iso-codes-team/iso-codes/-/archive/v4.17.0/iso-codes-v4.17.0.tar.gz |
| SHA-256 | `dd5ca13db77ec6dd1cc25f5c0184290a870ec1fed245d8e39a04ff34f59076c3` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_iso_codes`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add iso-codes
apk add iso-codes-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-iso-codes && /usr/local/bin/qnx-port-tests/test_iso_codes    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install iso-codes`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild iso-codes` performs exactly the same steps (2–5) in one command.

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
URL=https://salsa.debian.org/iso-codes-team/iso-codes/-/archive/v4.17.0/iso-codes-v4.17.0.tar.gz
mkdir -p dist src/iso-codes
curl -fL -o dist/$(basename $URL) $URL
echo "dd5ca13db77ec6dd1cc25f5c0184290a870ec1fed245d8e39a04ff34f59076c3  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/iso-codes --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/iso-codes && cd build/$QNX_ARCH/iso-codes
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/iso-codes/configure --host=$QNX_TRIPLE --prefix=/usr/local
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ISO country, language and currency code lists (data; needed by Epiphany).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH iso-codes`.

### 5. What it installs

1161 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/iso-codes` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa iso-codes
ls dists/8.0/*/ | grep '^iso-codes-'
```

This writes `iso-codes-<ver>-r0.tar.gz` (run time) and `iso-codes-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add iso-codes
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/iso-codes-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/iso-codes-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_iso_codes`](../../tests/src/) links iso-codes the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-iso-codes`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-iso-codes
/usr/local/bin/qnx-port-tests/test_iso_codes
```

```
== iso-codes ==
  ok   ...
PASS iso-codes
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_iso_codes` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_iso_codes`](../../tests/src/) -- a program linked against iso-codes the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - share/iso-codes/json installed
   - ISO 3166-1: 249 or more countries
   - DE: Germany, DEU, 276
   - numeric 392 -> JP; CAN -> Canada
   - FR official name: French Republic
   - ISO 3166-2: DE-BY Bayern, US-CA California
   - ISO 4217: EUR 978, JPY Yen
   - ISO 639-2: fra = fr, fre, French
   - ISO 639-3: 7000+ languages, deu German
   - ISO 15924: Latn 215, Cyrl Cyrillic
   - de catalogue: Germany -> Deutschland
   - fr catalogue: German -> Allemand
   - fr catalogue: Euro -> Euro

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | iso-codes 4.17.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/iso-codes-4.17.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | iso-codes 4.17.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/iso-codes-4.17.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.17.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add iso-codes=<version>`), sources in `ported/iso-codes/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates iso-codes`; port one with `scripts/port-update iso-codes`.

<details><summary>recipe.sh</summary>

```bash
# ISO country, language and currency code lists (data; needed by Epiphany).
VERSION=4.17.0
TARBALL=https://salsa.debian.org/iso-codes-team/iso-codes/-/archive/v4.17.0/iso-codes-v4.17.0.tar.gz
SHA256=dd5ca13db77ec6dd1cc25f5c0184290a870ec1fed245d8e39a04ff34f59076c3
BUILDSYS=autotools
CONF_ARGS_EXTRA=( )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
