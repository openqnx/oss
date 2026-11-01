# poco 1.15.4

> Cross-platform C++ libraries for network-based applications

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/pocoproject/poco.git (tag `poco-1.15.4-release`) |
| Build system | CMake |
| QNX patches | 0 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`pcre2`](../pcre2/) [`utf8proc`](../utf8proc/) [`expat`](../expat/) |
| Run-time dependencies | [`expat`](../expat/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`utf8proc`](../utf8proc/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_poco`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `poco` (programs, shared libraries, run-time data) and `poco-dev` (headers, pkg-config/CMake files, static libraries; needs `poco`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`poco`](https://openqnx.github.io/oss/package/x86_64/poco.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/poco.html)) · [`poco-dev`](https://openqnx.github.io/oss/package/x86_64/poco-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/poco-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add poco` (dependencies come along); one version: `apk add poco=<version>` | `apk del poco` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install poco` (dependencies first) | `oss-get remove poco` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`poco-dev` the same way, by its own name (`apk add poco-dev`, `oss-get install poco-dev`, by hand with its own file and list `poco-dev.list`); remove it before `poco`, which it needs.

```sh
# on the QNX target, as root
apk add poco poco-dev
apk add qnx-test-poco && /usr/local/bin/qnx-port-tests/test_poco    # its on-target test, from tests-8.0-<arch>-m-z
apk del poco poco-dev qnx-test-poco
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/poco.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/poco.list
rm var/lib/manual/poco.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild poco` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (6 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib pcre2 utf8proc expat
```

<details><summary>Full build chain (6)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`utf8proc`](../utf8proc/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b poco-1.15.4-release --recurse-submodules --shallow-submodules \
    https://github.com/pocoproject/poco.git src/poco
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/poco -B build/$QNX_ARCH/poco -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DPOCO_UNBUNDLED=ON \
      -DENABLE_TESTS=OFF \
      -DENABLE_SAMPLES=OFF \
      -DENABLE_FOUNDATION=ON \
      -DENABLE_UTIL=ON \
      -DENABLE_NET=ON \
      -DENABLE_XML=ON \
      -DENABLE_JSON=ON \
      -DENABLE_CRYPTO=ON \
      -DENABLE_NETSSL=OFF \
      -DENABLE_JWT=OFF \
      -DENABLE_DATA=OFF \
      -DENABLE_DATA_SQLITE=OFF \
      -DENABLE_DATA_MYSQL=OFF \
      -DENABLE_DATA_POSTGRESQL=OFF \
      -DENABLE_DATA_ODBC=OFF \
      -DENABLE_ZIP=OFF \
      -DENABLE_ENCODINGS=OFF \
      -DENABLE_MONGODB=OFF \
      -DENABLE_REDIS=OFF \
      -DENABLE_PROMETHEUS=OFF \
      -DENABLE_ACTIVERECORD=OFF \
      -DENABLE_ACTIVERECORD_COMPILER=OFF \
      -DENABLE_PAGECOMPILER=OFF \
      -DENABLE_PAGECOMPILER_FILE2PAGE=OFF \
      -DENABLE_APACHECONNECTOR=OFF \
      -DENABLE_FASTLOGGER=OFF \
      -DENABLE_PDF=OFF \
      -DENABLE_SEVENZIP=OFF
cmake --build build/$QNX_ARCH/poco -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/poco
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The POCO C++ Libraries, a core module set: Foundation, Util, Net, XML, JSON and Crypto (on the ported OpenSSL). Unbundled: zlib, PCRE2, utf8proc and expat are the workspace's, not copies compiled into Foundation and XML. Left out: NetSSL, JWT, Data (SQLite/MySQL/PostgreSQL/ODBC), Zip, Encodings, MongoDB, Redis, Prometheus, ActiveRecord, PageCompiler, the tools and the samples. FastLogger (Quill) disables itself: Quill knows no QNX. Upstream's CMake already has the QNX settings (_QNX_SOURCE, poll()).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH poco`.

### 5. What it installs

644 files under `/usr/local`: 12 shared-library entries, 606 headers.

```
/usr/local/lib/libPocoCrypto.so
/usr/local/lib/libPocoCrypto.so.124
/usr/local/lib/libPocoFoundation.so
/usr/local/lib/libPocoFoundation.so.124
/usr/local/lib/libPocoJSON.so
/usr/local/lib/libPocoJSON.so.124
/usr/local/lib/libPocoNet.so
/usr/local/lib/libPocoNet.so.124
/usr/local/lib/libPocoUtil.so
/usr/local/lib/libPocoUtil.so.124
/usr/local/lib/libPocoXML.so
/usr/local/lib/libPocoXML.so.124
```

The complete list is `stage/<arch>/.manifest/poco` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa poco
ls dists/8.0/*/ | grep '^poco-'
```

This writes `poco-<ver>-r0.tar.gz` (run time) and `poco-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add poco
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/poco-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/poco-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_poco`](../../tests/src/) links poco the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-poco`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-poco
/usr/local/bin/qnx-port-tests/test_poco
```

```
== poco ==
  ok   ...
PASS poco
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_poco` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_poco`](../../tests/src/) -- a program linked against poco the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - MD5(abc)
   - SHA1(abc)
   - Crypto SHA-256(abc) via OpenSSL
   - AES-256-CBC = openssl enc (base64 Q8r1+PKGrD6M+hU4mwup4w==)
   - AES-256-CBC decrypt
   - RSA-1024 SHA-256 signature verifies, not for other text
   - DateTimeParser ISO 8601 -> epoch 1234567890, Friday
   - DateTimeFormatter HTTP format
   - RegularExpression (PCRE2) captures
   - UTF8::toUpper (utf8proc)
   - gzip Deflating/InflatingStream round trip (5003 -> <200 bytes)
   - FileOutputStream writes 12 bytes
   - File::list
   - File::remove(recursive)
   - ThreadPool: 4 runnables x 1000 increments
   - Util::PropertyFileConfiguration
   - Util::XMLConfiguration paths
   - XML DOMParser: elements, attribute, text
   - JSON parse: string, number, array, bool
   - JSON stringify
   - HTTP GET on 127.0.0.1
   - HTTP POST on the same keep-alive session
   - DNS::resolve(127.0.0.1): a name, and only loopback addresses
   - DNS::hostByName(127.0.0.1) = 127.0.0.1

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | poco 1.15.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/poco-1.15.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | poco 1.15.4-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/poco-1.15.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.15.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add poco=<version>`), sources in `ported/poco/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates poco`; port one with `scripts/port-update poco`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.15.4; GIT=https://github.com/pocoproject/poco.git; REF=poco-1.15.4-release
DEPS="openssl zlib pcre2 utf8proc expat"
# The POCO C++ Libraries, a core module set: Foundation, Util, Net, XML, JSON
# and Crypto (on the ported OpenSSL). Unbundled: zlib, PCRE2, utf8proc and
# expat are the workspace's, not copies compiled into Foundation and XML.
# Left out: NetSSL, JWT, Data (SQLite/MySQL/PostgreSQL/ODBC), Zip, Encodings,
# MongoDB, Redis, Prometheus, ActiveRecord, PageCompiler, the tools and the
# samples. FastLogger (Quill) disables itself: Quill knows no QNX.
# Upstream's CMake already has the QNX settings (_QNX_SOURCE, poll()).
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DPOCO_UNBUNDLED=ON -DENABLE_TESTS=OFF -DENABLE_SAMPLES=OFF
  -DENABLE_FOUNDATION=ON -DENABLE_UTIL=ON -DENABLE_NET=ON -DENABLE_XML=ON -DENABLE_JSON=ON -DENABLE_CRYPTO=ON
  -DENABLE_NETSSL=OFF -DENABLE_JWT=OFF -DENABLE_DATA=OFF -DENABLE_DATA_SQLITE=OFF -DENABLE_DATA_MYSQL=OFF
  -DENABLE_DATA_POSTGRESQL=OFF -DENABLE_DATA_ODBC=OFF -DENABLE_ZIP=OFF -DENABLE_ENCODINGS=OFF
  -DENABLE_MONGODB=OFF -DENABLE_REDIS=OFF -DENABLE_PROMETHEUS=OFF -DENABLE_ACTIVERECORD=OFF
  -DENABLE_ACTIVERECORD_COMPILER=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF
  -DENABLE_APACHECONNECTOR=OFF -DENABLE_FASTLOGGER=OFF -DENABLE_PDF=OFF -DENABLE_SEVENZIP=OFF )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
