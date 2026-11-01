# ippusbxd 1.34

> Daemon for IPP over USB printer support

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/OpenPrinting/ippusbxd.git (tag `1.34`) |
| Build system | CMake |
| QNX patches | 1 |
| Build dependencies | [`libusb`](../libusb/) [`avahi`](../avahi/) [`libxml2`](../libxml2/) [`cups`](../cups/) |
| Run-time dependencies | [`avahi`](../avahi/) [`cups`](../cups/) [`libusb`](../libusb/) [`libxml2`](../libxml2/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_ippusbxd`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `ippusbxd` (programs, shared libraries, run-time data) and `ippusbxd-dev` (headers, pkg-config/CMake files, static libraries; needs `ippusbxd`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`ippusbxd`](https://openqnx.github.io/oss/package/x86_64/ippusbxd.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ippusbxd.html)) · [`ippusbxd-dev`](https://openqnx.github.io/oss/package/x86_64/ippusbxd-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/ippusbxd-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add ippusbxd` (dependencies come along); one version: `apk add ippusbxd=<version>` | `apk del ippusbxd` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install ippusbxd` (dependencies first) | `oss-get remove ippusbxd` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`ippusbxd-dev` the same way, by its own name (`apk add ippusbxd-dev`, `oss-get install ippusbxd-dev`, by hand with its own file and list `ippusbxd-dev.list`); remove it before `ippusbxd`, which it needs.

```sh
# on the QNX target, as root
apk add ippusbxd ippusbxd-dev
apk add qnx-test-ippusbxd && /usr/local/bin/qnx-port-tests/test_ippusbxd    # its on-target test, from tests-8.0-<arch>
apk del ippusbxd ippusbxd-dev qnx-test-ippusbxd
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/ippusbxd.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/ippusbxd.list
rm var/lib/manual/ippusbxd.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild ippusbxd` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (22 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libusb avahi libxml2 cups
```

<details><summary>Full build chain (22)</summary>

[`avahi`](../avahi/) [`bzip2`](../bzip2/) [`cups`](../cups/) [`dbus`](../dbus/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libdaemon`](../libdaemon/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libusb`](../libusb/) [`libxcrypt`](../libxcrypt/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 1.34 --recurse-submodules --shallow-submodules \
    https://github.com/OpenPrinting/ippusbxd.git src/ippusbxd
```

The project lives in the `src/` subdirectory of the repository.

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/ippusbxd < ports/ippusbxd/patches/0001-tcp-no-variable-named-true.patch
```

**[`0001-tcp-no-variable-named-true.patch`](patches/0001-tcp-no-variable-named-true.patch)** — Do not name a variable 'true'.

> tcp.c names a local variable "true" (the SO_REUSEADDR option value). On QNX
> <stdbool.h> is pulled in by the system headers and "true" is a macro there,
> so the declaration reads "int 1 = 1;". Rename the variable.

Touches: `src/tcp.c` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/ippusbxd/src; B=build/$QNX_ARCH/ippusbxd; mkdir -p $B
```

```bash
do_install() {
  install -d "$STAGE$PREFIX/sbin" "$STAGE$PREFIX/share/man/man8" &&
  install -m 755 "$B/ippusbxd" "$STAGE$PREFIX/sbin/" &&
  install -m 644 "$SRCROOT/doc/ippusbxd.8" "$STAGE$PREFIX/share/man/man8/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- ippusbxd: IPP-over-USB daemon -- makes a USB printer that speaks IPP over USB (IPP Everywhere / AirPrint class) reachable as a network IPP printer on localhost, advertised through Avahi.
- The project has no install rule (its makefile only builds); install the daemon and its manual.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH ippusbxd`.

### 5. What it installs

2 files under `/usr/local`, 1 programs.

```
/usr/local/sbin/ippusbxd
```

The complete list is `stage/<arch>/.manifest/ippusbxd` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa ippusbxd
ls dists/8.0/*/ | grep '^ippusbxd-'
```

This writes `ippusbxd-<ver>-r0.tar.gz` (run time) and `ippusbxd-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add ippusbxd
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/ippusbxd-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/ippusbxd-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_ippusbxd`](../../tests/src/) links ippusbxd the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-ippusbxd`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-ippusbxd
/usr/local/bin/qnx-port-tests/test_ippusbxd
```

```
== ippusbxd ==
  ok   ...
PASS ippusbxd
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_ippusbxd` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_ippusbxd`](../../tests/src/) -- a program linked against ippusbxd the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | ippusbxd 1.34-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/ippusbxd-1.34-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | ippusbxd 1.34-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/ippusbxd-1.34-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.34**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add ippusbxd=<version>`), sources in `ported/ippusbxd/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates ippusbxd`; port one with `scripts/port-update ippusbxd`.

<details><summary>recipe.sh</summary>

```bash
# ippusbxd: IPP-over-USB daemon -- makes a USB printer that speaks IPP over
# USB (IPP Everywhere / AirPrint class) reachable as a network IPP printer on
# localhost, advertised through Avahi.
VERSION=1.34; GIT=https://github.com/OpenPrinting/ippusbxd.git; REF=1.34
SUBDIR=src
BUILDSYS=cmake
DEPS="libusb avahi libxml2 cups"
# The project has no install rule (its makefile only builds); install the
# daemon and its manual.
do_install() {
  install -d "$STAGE$PREFIX/sbin" "$STAGE$PREFIX/share/man/man8" &&
  install -m 755 "$B/ippusbxd" "$STAGE$PREFIX/sbin/" &&
  install -m 644 "$SRCROOT/doc/ippusbxd.8" "$STAGE$PREFIX/share/man/man8/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
