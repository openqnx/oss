# apr-util 1.6.5

> Apache Portable Runtime utility library

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.apache.org/dist/apr/apr-util-1.6.5.tar.bz2 |
| SHA-256 | `96de1dd6f6a0476d2d2e7964926d8c1ddc3bb0e210e1b1812d3ba5a454a392e2` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`apr`](../apr/) [`expat`](../expat/) [`openssl`](../openssl/) [`sqlite`](../sqlite/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`apr`](../apr/) [`expat`](../expat/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`sqlite`](../sqlite/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_apr_util`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `apr-util` (programs, shared libraries, run-time data) and `apr-util-dev` (headers, pkg-config/CMake files, static libraries; needs `apr-util`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`apr-util`](https://openqnx.github.io/oss/package/x86_64/apr-util.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/apr-util.html)) · [`apr-util-dev`](https://openqnx.github.io/oss/package/x86_64/apr-util-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/apr-util-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add apr-util` (dependencies come along); one version: `apk add apr-util=<version>` | `apk del apr-util` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install apr-util` (dependencies first) | `oss-get remove apr-util` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`apr-util-dev` the same way, by its own name (`apk add apr-util-dev`, `oss-get install apr-util-dev`, by hand with its own file and list `apr-util-dev.list`); remove it before `apr-util`, which it needs.

```sh
# on the QNX target, as root
apk add apr-util apr-util-dev
apk add qnx-test-apr-util && /usr/local/bin/qnx-port-tests/test_apr_util    # its on-target test, from tests-8.0-<arch>
apk del apr-util apr-util-dev qnx-test-apr-util
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/apr-util.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/apr-util.list
rm var/lib/manual/apr-util.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild apr-util` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (7 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild apr expat openssl sqlite libiconv libxcrypt
```

<details><summary>Full build chain (7)</summary>

[`apr`](../apr/) [`expat`](../expat/) [`libiconv`](../libiconv/) [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`sqlite`](../sqlite/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.apache.org/dist/apr/apr-util-1.6.5.tar.bz2
mkdir -p dist src/apr-util
curl -fL -o dist/$(basename $URL) $URL
echo "96de1dd6f6a0476d2d2e7964926d8c1ddc3bb0e210e1b1812d3ba5a454a392e2  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/apr-util --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/apr-util; B=build/$QNX_ARCH/apr-util; mkdir -p $B
```

```bash
do_build() {
  sed -i -E "s#^(apr_builddir|apr_builders|top_builddir)=$PREFIX/share/apr-1/build#\1=$STAGE$PREFIX/share/apr-1/build#" "$B/build/rules.mk" &&
  make -C "$B" -j"$(nproc)"
}
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i -e 's|^APU_SOURCE_DIR=.*|APU_SOURCE_DIR=""|' -e 's|^APU_BUILD_DIR=.*|APU_BUILD_DIR=""|' "$STAGE$PREFIX/bin/apu-1-config"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- apr-1-config run from staging answers with staging paths (its "crosscompile" mode); the build rules and libtool come from apr. XML via the ported expat; apr_crypto with OpenSSL and the SQLite 3 DBD driver, both built as modules in lib/apr-util-1/ that apr-util dlopen()s from $PREFIX/lib/apr-util-1. No LDAP, PostgreSQL, MySQL, ODBC, Berkeley DB or gdbm (the built-in sdbm is the DBM). QNX's libc has neither iconv nor crypt(): apr_xlate uses GNU libiconv, apr_password_validate libxcrypt.
- configure copies apr's apr_rules.mk, which names apr's helpers (libtool, make_exports.awk) where apr installs them on the target; the build takes them from staging.
- As for apr-1-config: the source and build trees are only for uninstalled use.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH apr-util`.

### 5. What it installs

45 files under `/usr/local`: 3 shared-library entries, 1 programs, 35 headers.

```
/usr/local/lib/libaprutil-1.so
/usr/local/lib/libaprutil-1.so.0
/usr/local/lib/libaprutil-1.so.0.6.5
/usr/local/bin/apu-1-config
```

The complete list is `stage/<arch>/.manifest/apr-util` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa apr-util
ls dists/8.0/*/ | grep '^apr-util-'
```

This writes `apr-util-<ver>-r0.tar.gz` (run time) and `apr-util-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add apr-util
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/apr-util-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/apr-util-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_apr_util`](../../tests/src/) links apr-util the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-apr-util`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-apr-util
/usr/local/bin/qnx-port-tests/test_apr_util
```

```
== apr-util ==
  ok   ...
PASS apr-util
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_apr_util` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_apr_util`](../../tests/src/) -- a program linked against apr-util the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - apr_base64_encode(QNX) = UU5Y
   - apr_md5(abc)
   - apr_sha1(abc)
   - apr_md5_encode $apr1$ hash validates the right password only
   - apr_password_validate: SHA-512 crypt hash from openssl passwd -6 (system crypt)
   - apr_uuid_get + format: 36 characters
   - apr_uuid_parse round trip
   - two apr_uuid_get() differ
   - apr_uri_parse splits scheme/user/host/port/path/query/fragment
   - apr_date_parse_http(RFC 1123 date) = 784111777 s
   - apr_xml parses a document, root <cfg>
   - two <item> children, first text 'one', attribute version=2
   - mismatched tags are an error
   - apr_xlate_open(ISO-8859-1 <- UTF-8)
   - UTF-8 'caf\xc3\xa9!' -> 5 bytes Latin-1
   - apr_dbm_open_ex(SDBM)
   - apr_dbm_store
   - reopen + apr_dbm_fetch
   - apr_queue is FIFO
   - apr_crypto_get_driver(openssl): module loaded from lib/apr-util-1
   - AES-256-CBC key from a passphrase (PBKDF2, 4096 rounds), 16-byte IV
   - ciphertext = openssl enc -aes-256-cbc with the same key and IV
   - decrypts back to the message
   - decrypt
   - encrypt
   - ... and 5 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | apr-util 1.6.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/apr-util-1.6.5-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | apr-util 1.6.5-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/apr-util-1.6.5-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.6.5**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add apr-util=<version>`), sources in `ported/apr-util/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates apr-util`; port one with `scripts/port-update apr-util`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.6.5
TARBALL=https://archive.apache.org/dist/apr/apr-util-1.6.5.tar.bz2
SHA256=96de1dd6f6a0476d2d2e7964926d8c1ddc3bb0e210e1b1812d3ba5a454a392e2
BUILDSYS=autotools
DEPS="apr expat openssl sqlite libiconv libxcrypt"
# apr-1-config run from staging answers with staging paths (its
# "crosscompile" mode); the build rules and libtool come from apr.
# XML via the ported expat; apr_crypto with OpenSSL and the SQLite 3 DBD
# driver, both built as modules in lib/apr-util-1/ that apr-util dlopen()s
# from $PREFIX/lib/apr-util-1. No LDAP, PostgreSQL, MySQL, ODBC, Berkeley DB
# or gdbm (the built-in sdbm is the DBM). QNX's libc has neither iconv nor
# crypt(): apr_xlate uses GNU libiconv, apr_password_validate libxcrypt.
CONF_ARGS_EXTRA=( --with-apr="$STAGE$PREFIX/bin/apr-1-config" --with-expat="$STAGE$PREFIX"
  --with-crypto --with-openssl="$STAGE$PREFIX" --with-sqlite3="$STAGE$PREFIX"
  --without-ldap --without-pgsql --without-mysql --without-sqlite2 --without-odbc
  --without-berkeley-db --without-gdbm --without-ndbm --without-nss --without-commoncrypto )
# configure copies apr's apr_rules.mk, which names apr's helpers (libtool,
# make_exports.awk) where apr installs them on the target; the build takes
# them from staging.
do_build() {
  sed -i -E "s#^(apr_builddir|apr_builders|top_builddir)=$PREFIX/share/apr-1/build#\1=$STAGE$PREFIX/share/apr-1/build#" "$B/build/rules.mk" &&
  make -C "$B" -j"$(nproc)"
}
# As for apr-1-config: the source and build trees are only for uninstalled use.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i -e 's|^APU_SOURCE_DIR=.*|APU_SOURCE_DIR=""|' -e 's|^APU_BUILD_DIR=.*|APU_BUILD_DIR=""|' "$STAGE$PREFIX/bin/apu-1-config"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
