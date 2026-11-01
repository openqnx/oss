# gnupg 2.4.9

> GNU Privacy Guard 2: OpenPGP and S/MIME encryption and signing tools

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gnupg.org/ftp/gcrypt/gnupg/gnupg-2.4.9.tar.bz2 |
| SHA-256 | `dd17ab2e9a04fd79d39d853f599cbc852062ddb9ab52a4ddeb4176fd8b302964` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`libgpg-error`](../libgpg-error/) [`libgcrypt`](../libgcrypt/) [`libassuan`](../libassuan/) [`libksba`](../libksba/) [`npth`](../npth/) [`gnutls`](../gnutls/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) [`bzip2`](../bzip2/) [`readline`](../readline/) [`libiconv`](../libiconv/) |
| Run-time dependencies | [`bzip2`](../bzip2/) [`gnutls`](../gnutls/) [`libassuan`](../libassuan/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libksba`](../libksba/) [`ncurses`](../ncurses/) [`npth`](../npth/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_gnupg`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `gnupg` (programs, shared libraries, run-time data) and `gnupg-dev` (headers, pkg-config/CMake files, static libraries; needs `gnupg`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`gnupg`](https://openqnx.github.io/oss/package/x86_64/gnupg.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gnupg.html)) · [`gnupg-dev`](https://openqnx.github.io/oss/package/x86_64/gnupg-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/gnupg-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add gnupg` (dependencies come along); one version: `apk add gnupg=<version>` | `apk del gnupg` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install gnupg` (dependencies first) | `oss-get remove gnupg` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`gnupg-dev` the same way, by its own name (`apk add gnupg-dev`, `oss-get install gnupg-dev`, by hand with its own file and list `gnupg-dev.list`); remove it before `gnupg`, which it needs.

```sh
# on the QNX target, as root
apk add gnupg gnupg-dev
apk add qnx-test-gnupg && /usr/local/bin/qnx-port-tests/test_gnupg    # its on-target test, from tests-8.0-<arch>
apk del gnupg gnupg-dev qnx-test-gnupg
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/gnupg.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/gnupg.list
rm var/lib/manual/gnupg.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild gnupg` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (20 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libgpg-error libgcrypt libassuan libksba npth gnutls sqlite zlib bzip2 readline libiconv
```

<details><summary>Full build chain (20)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`libassuan`](../libassuan/) [`libgcrypt`](../libgcrypt/) [`libgpg-error`](../libgpg-error/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libksba`](../libksba/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`npth`](../npth/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://gnupg.org/ftp/gcrypt/gnupg/gnupg-2.4.9.tar.bz2
mkdir -p dist src/gnupg
curl -fL -o dist/$(basename $URL) $URL
echo "dd17ab2e9a04fd79d39d853f599cbc852062ddb9ab52a4ddeb4176fd8b302964  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/gnupg --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/gnupg < ports/gnupg/patches/0001-printable-os-name-qnx.patch
```

**[`0001-printable-os-name-qnx.patch`](patches/0001-printable-os-name-qnx.patch)** — Report the platform as QNX, not the build machine's uname

> configure names the platform after the build machine (uname -s) for any host it
> does not know, so a cross build for QNX printed "Linux" in the --version and
> --help output of every program. Name QNX for *-nto-qnx* hosts, in configure.ac
> and in the shipped configure (which the build does not regenerate).

Touches: `configure`, `configure.ac` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/gnupg && cd build/$QNX_ARCH/gnupg
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" \
$OLDPWD/src/gnupg/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --disable-doc \
    --disable-ldap \
    --disable-tpm2d \
    --disable-ccid-driver \
    --disable-nls \
    --disable-tests \
    --enable-sqlite \
    --enable-gnutls \
    --with-libgpg-error-prefix="$STAGE$PREFIX" \
    --with-libgcrypt-prefix="$STAGE$PREFIX" \
    --with-libassuan-prefix="$STAGE$PREFIX" \
    --with-ksba-prefix="$STAGE$PREFIX" \
    --with-npth-prefix="$STAGE$PREFIX" \
    --with-zlib="$STAGE$PREFIX" \
    --with-bzip2="$STAGE$PREFIX" \
    --with-readline="$STAGE$PREFIX"
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- 2.4 is the newest release series this tree's libraries can carry: GnuPG 2.5 needs libgpg-error >= 1.56 and libgcrypt >= 1.12 (ported here: 1.50, 1.11).
- configure probes PATH for gpgrt-config and would otherwise pick up the build host's copy, which reports the host's libgpg-error rather than ours.
- No LDAP client library and no TPM2 stack here: dirmngr goes without LDAP keyserver support, and tpm2d is not built. The internal CCID driver needs libusb; scdaemon still talks to readers through PC/SC (loaded at run time). NLS off: messages stay English, and no catalogue path is compiled in. --disable-tests: the test-suite interpreter (gpgscm) needs gcc's -imacros, which qcc rejects, and the suite only runs on the build machine anyway.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH gnupg`.

### 5. What it installs

30 files under `/usr/local`, 19 programs.

```
/usr/local/bin/dirmngr
/usr/local/bin/dirmngr-client
/usr/local/bin/gpg
/usr/local/bin/gpg-agent
/usr/local/bin/gpg-card
/usr/local/bin/gpg-connect-agent
/usr/local/bin/gpg-mail-tube
/usr/local/bin/gpg-wks-client
/usr/local/bin/gpg-wks-server
/usr/local/bin/gpgconf
/usr/local/bin/gpgparsemail
/usr/local/bin/gpgsm
/usr/local/bin/gpgsplit
/usr/local/bin/gpgtar
/usr/local/bin/gpgv
/usr/local/bin/kbxutil
...
```

The complete list is `stage/<arch>/.manifest/gnupg` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa gnupg
ls dists/8.0/*/ | grep '^gnupg-'
```

This writes `gnupg-<ver>-r0.tar.gz` (run time) and `gnupg-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add gnupg
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/gnupg-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/gnupg-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_gnupg`](../../tests/src/) links gnupg the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-gnupg`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-gnupg
/usr/local/bin/qnx-port-tests/test_gnupg
```

```
== gnupg ==
  ok   ...
PASS gnupg
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_gnupg` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_gnupg`](../../tests/src/) -- a program linked against gnupg the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | gnupg 2.4.9-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/gnupg-2.4.9-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | gnupg 2.4.9-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/gnupg-2.4.9-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.4.9**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add gnupg=<version>`), sources in `ported/gnupg/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates gnupg`; port one with `scripts/port-update gnupg`.

<details><summary>recipe.sh</summary>

```bash
# 2.4 is the newest release series this tree's libraries can carry: GnuPG 2.5
# needs libgpg-error >= 1.56 and libgcrypt >= 1.12 (ported here: 1.50, 1.11).
VERSION=2.4.9
TARBALL=https://gnupg.org/ftp/gcrypt/gnupg/gnupg-2.4.9.tar.bz2
SHA256=dd17ab2e9a04fd79d39d853f599cbc852062ddb9ab52a4ddeb4176fd8b302964
BUILDSYS=autotools
DEPS="libgpg-error libgcrypt libassuan libksba npth gnutls sqlite zlib bzip2 readline libiconv"
# configure probes PATH for gpgrt-config and would otherwise pick up the build
# host's copy, which reports the host's libgpg-error rather than ours.
CONF_ENV=( GPGRT_CONFIG="$STAGE$PREFIX/bin/gpgrt-config" )
# No LDAP client library and no TPM2 stack here: dirmngr goes without LDAP
# keyserver support, and tpm2d is not built. The internal CCID driver needs
# libusb; scdaemon still talks to readers through PC/SC (loaded at run time).
# NLS off: messages stay English, and no catalogue path is compiled in.
# --disable-tests: the test-suite interpreter (gpgscm) needs gcc's -imacros,
# which qcc rejects, and the suite only runs on the build machine anyway.
CONF_ARGS_EXTRA=( --disable-doc --disable-ldap --disable-tpm2d --disable-ccid-driver
                  --disable-nls --disable-tests --enable-sqlite --enable-gnutls
                  --with-libgpg-error-prefix="$STAGE$PREFIX"
                  --with-libgcrypt-prefix="$STAGE$PREFIX"
                  --with-libassuan-prefix="$STAGE$PREFIX"
                  --with-ksba-prefix="$STAGE$PREFIX"
                  --with-npth-prefix="$STAGE$PREFIX"
                  --with-zlib="$STAGE$PREFIX" --with-bzip2="$STAGE$PREFIX"
                  --with-readline="$STAGE$PREFIX" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
