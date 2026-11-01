# gpgme 2.2.0

> GnuPG Made Easy

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gnupg.org/ftp/gcrypt/gpgme/gpgme-2.2.0.tar.bz2 |
| SHA-256 | `7160e80e84dafd00d956c84891c533bb7ab16a6a54fbe1574b2f3acf0496977b` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libgpg-error`](../libgpg-error/) [`libassuan`](../libassuan/) |
| Run-time dependencies | [`libassuan`](../libassuan/) [`libgpg-error`](../libgpg-error/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gpgme`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gpgme` (programs, shared libraries, run-time data) and `gpgme-dev` (headers, pkg-config/CMake files, static libraries; needs `gpgme`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gpgme`](https://openqnx.github.io/oss/package/x86_64/gpgme.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gpgme.html)) · [`gpgme-dev`](https://openqnx.github.io/oss/package/x86_64/gpgme-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gpgme-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gpgme` (dependencies come along); one version: `apk add gpgme=<version>` | `apk del gpgme` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gpgme` (dependencies first) | `oss-get remove gpgme` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gpgme-dev` the same way, by its own name (`apk add gpgme-dev`, `oss-get install gpgme-dev`, by hand with its own file and list `gpgme-dev.list`); remove it before `gpgme`, which it needs.

```sh
# on the QNX target, as root
apk add gpgme gpgme-dev
apk add qnx-test-gpgme && /usr/local/bin/qnx-port-tests/test_gpgme    # its on-target test, from tests-8.0-<arch>
apk del gpgme gpgme-dev qnx-test-gpgme
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gpgme.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gpgme.list
rm var/lib/manual/gpgme.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gpgme` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libgpg-error libassuan
```

<details><summary>Full build chain (6)</summary>

[`gettext`](../gettext/) [`libassuan`](../libassuan/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/)
</details>

### 2. Get the upstream source

```bash
URL=https://gnupg.org/ftp/gcrypt/gpgme/gpgme-2.2.0.tar.bz2
mkdir -p dist src/gpgme
curl -fL -o dist/$(basename $URL) $URL
echo "7160e80e84dafd00d956c84891c533bb7ab16a6a54fbe1574b2f3acf0496977b  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gpgme --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gpgme && cd build/$QNX_ARCH/gpgme
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" \
$OLDPWD/src/gpgme/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gpg-test \
    --disable-gpgsm-test \
    --disable-gpgconf-test \
    --disable-g13-test \
    --with-libgpg-error-prefix="$STAGE$PREFIX" \
    --with-libassuan-prefix="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- GPGME, GnuPG Made Easy: the C library through which applications use GnuPG (it runs gpg, gpgsm and gpgconf from the gnupg component and talks to them over pipes and Assuan).
- configure probes PATH for gpgrt-config and would otherwise pick up the build host's copy, which reports the host's libgpg-error rather than ours.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gpgme`.

### 5. What it installs

17 files under `/usr/local`: 2 shared-library entries, 4 programs, 1 headers.

```
/usr/local/lib/libgpgme.so
/usr/local/lib/libgpgme.so.47
/usr/local/bin/gnupg-key-manage
/usr/local/bin/gpgme-config
/usr/local/bin/gpgme-json
/usr/local/bin/gpgme-tool
```

The complete list is `stage/<arch>/.manifest/gpgme` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gpgme
ls dists/8.0/*/ | grep '^gpgme-'
```

This writes `gpgme-<ver>-r0.tar.gz` (run time) and `gpgme-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gpgme
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gpgme-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gpgme-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gpgme`](../../tests/src/) links gpgme the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gpgme`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gpgme
/usr/local/bin/qnx-port-tests/test_gpgme
```

```
== gpgme ==
  ok   ...
PASS gpgme
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gpgme` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gpgme`](../../tests/src/) -- a program linked against gpgme the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - gpgme_check_version: 2.2.0
   - OpenPGP engine usable
   - scratch GNUPGHOMEs
   - OpenPGP engine: gpg 2.4.9
   - context on the first scratch home
   - ed25519 key created without passphrase (40-hex fingerprint)
   - gpgme_get_key by fingerprint
   - cv25519 encryption subkey added
   - keylist: EdDSA signing primary, ECDH cv25519 encryption subkey
   - keylist: the user id
   - encrypted to the key (ASCII armour)
   - decrypted: the plaintext back
   - decrypt result: recipient is the cv25519 subkey
   - detached signature made
   - signature verifies: valid, by the key's fingerprint
   - signature algorithm EdDSA
   - altered data: GPG_ERR_BAD_SIGNATURE
   - clear-text signature contains the message
   - public key exported
   - imported into the second home: 1 key, same fingerprint
   - second context
   - symmetric encryption (passphrase callback) decrypted in the other home
   - symmetric encryption

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gpgme 2.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gpgme-2.2.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gpgme 2.2.0-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gpgme-2.2.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.2.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gpgme=<version>`), sources in `ported/gpgme/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gpgme`; port one with `scripts/port-update gpgme`.

<details><summary>recipe.sh</summary>

```bash
# GPGME, GnuPG Made Easy: the C library through which applications use GnuPG
# (it runs gpg, gpgsm and gpgconf from the gnupg component and talks to them
# over pipes and Assuan).
VERSION=2.2.0
TARBALL=https://gnupg.org/ftp/gcrypt/gpgme/gpgme-2.2.0.tar.bz2
SHA256=7160e80e84dafd00d956c84891c533bb7ab16a6a54fbe1574b2f3acf0496977b
BUILDSYS=autotools
DEPS="libgpg-error libassuan"
RUNTIME_DEPS="gnupg"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gpg-test --disable-gpgsm-test
                  --disable-gpgconf-test --disable-g13-test
                  --with-libgpg-error-prefix="$STAGE$PREFIX"
                  --with-libassuan-prefix="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
