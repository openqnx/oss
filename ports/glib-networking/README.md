# glib-networking 2.80.1

> TLS (GnuTLS) and proxy support modules for GLib's GIO

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://gitlab.gnome.org/GNOME/glib-networking.git (tag `2.80.1`) |
| Build system | Meson |
| QNX patches | 0 |
| Build dependencies | [`glib`](../glib/) [`gnutls`](../gnutls/) [`libproxy`](../libproxy/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`gnutls`](../gnutls/) [`libproxy`](../libproxy/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_glib_networking`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `glib-networking` (programs, shared libraries, run-time data) and `glib-networking-dev` (headers, pkg-config/CMake files, static libraries; needs `glib-networking`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`glib-networking`](https://openqnx.github.io/oss/package/x86_64/glib-networking.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glib-networking.html)) · [`glib-networking-dev`](https://openqnx.github.io/oss/package/x86_64/glib-networking-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/glib-networking-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add glib-networking` (dependencies come along); one version: `apk add glib-networking=<version>` | `apk del glib-networking` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install glib-networking` (dependencies first) | `oss-get remove glib-networking` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`glib-networking-dev` the same way, by its own name (`apk add glib-networking-dev`, `oss-get install glib-networking-dev`, by hand with its own file and list `glib-networking-dev.list`); remove it before `glib-networking`, which it needs.

```sh
# on the QNX target, as root
apk add glib-networking glib-networking-dev
apk add qnx-test-glib-networking && /usr/local/bin/qnx-port-tests/test_glib_networking    # its on-target test
apk del glib-networking glib-networking-dev qnx-test-glib-networking
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/glib-networking.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/glib-networking.list
rm var/lib/manual/glib-networking.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild glib-networking` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (21 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gnutls libproxy
```

<details><summary>Full build chain (21)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`c-ares`](../c-ares/) [`curl`](../curl/) [`gettext`](../gettext/) [`glib`](../glib/) [`gmp`](../gmp/) [`gnutls`](../gnutls/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libidn2`](../libidn2/) [`libproxy`](../libproxy/) [`libtasn1`](../libtasn1/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`nettle`](../nettle/) [`nghttp2`](../nghttp2/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 2.80.1 --recurse-submodules --shallow-submodules \
    https://gitlab.gnome.org/GNOME/glib-networking.git src/glib-networking
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/glib-networking src/glib-networking \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dgnutls=enabled \
      -Dopenssl=disabled \
      -Dlibproxy=enabled \
      -Dgnome_proxy=disabled \
      -Dinstalled_tests=false
ninja -C build/$QNX_ARCH/glib-networking
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/glib-networking install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- glib-networking: TLS (GnuTLS) and proxy support for GIO.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH glib-networking`.

### 5. What it installs

81 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/glib-networking` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa glib-networking
ls dists/8.0/*/ | grep '^glib-networking-'
```

This writes `glib-networking-<ver>-r0.tar.gz` (run time) and `glib-networking-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add glib-networking
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/glib-networking-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/glib-networking-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_glib_networking`](../../tests/src/) links glib-networking the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-glib-networking`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-glib-networking
/usr/local/bin/qnx-port-tests/test_glib_networking
```

```
== glib-networking ==
  ok   ...
PASS glib-networking
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_glib_networking` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_glib_networking`](../../tests/src/) -- a program linked against glib-networking the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - a TLS-capable backend is loaded
   - it is glib-networking's GnuTLS backend
   - certificate + key parsed from PEM
   - certificate subject name
   - listening on 127.0.0.1
   - TLS handshake completes
   - self-signed certificate flagged as unknown CA
   - ping/pong over TLS
   - client sees the server's certificate
   - TLS 1.2 or 1.3 negotiated
   - file database with the certificate as anchor
   - second session with the trust anchor
   - certificate validates for localhost against the anchor

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | glib-networking 2.80.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/glib-networking-2.80.1-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | glib-networking 2.80.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/glib-networking-2.80.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | glib-networking 2.80.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/glib-networking-2.80.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | glib-networking 2.80.1-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/glib-networking-2.80.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.80.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add glib-networking=<version>`), sources in `ported/glib-networking/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates glib-networking`; port one with `scripts/port-update glib-networking`.

<details><summary>recipe.sh</summary>

```bash
# glib-networking: TLS (GnuTLS) and proxy support for GIO.
VERSION=2.80.1; GIT=https://gitlab.gnome.org/GNOME/glib-networking.git; REF=2.80.1
DEPS="glib gnutls libproxy"
BUILDSYS=meson
MESON_ARGS_EXTRA=( -Dgnutls=enabled -Dopenssl=disabled -Dlibproxy=enabled -Dgnome_proxy=disabled -Dinstalled_tests=false )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
