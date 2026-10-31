# man-db 2.13.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz |
| SHA-256 | `8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`libpipeline`](../libpipeline/) [`gdbm`](../gdbm/) [`groff`](../groff/) [`libiconv`](../libiconv/) [`zlib`](../zlib/) |
| Run-time dependencies | [`gdbm`](../gdbm/) [`libiconv`](../libiconv/) [`libpipeline`](../libpipeline/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add man-db
apk add man-db-dev    # only for building on the target: headers, .pc, CMake files
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install man-db`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild man-db` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libpipeline gdbm groff libiconv zlib
```

### 2. Get the upstream source

```bash
URL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
mkdir -p dist src/man-db
curl -fL -o dist/$(basename $URL) $URL
echo "8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/man-db --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/man-db; B=build/$QNX_ARCH/man-db; mkdir -p $B
```

```bash
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
- The "manual" directory typesets man-db's user guide with groff's -me macros on the build host, where they are often not installed (Debian's groff-base lacks them). The guide is not needed to run man; leave that directory out of the generated top-level Makefile. (Not with SUBDIRS=... on the make command line: that reaches every sub-make too, and gl/lib then looks for "docs".)

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH man-db`.

### 5. What it installs

29 files under `/usr/local`, 9 programs.

```
/usr/local/bin/apropos
/usr/local/bin/catman
/usr/local/bin/lexgrog
/usr/local/bin/man
/usr/local/bin/man-recode
/usr/local/bin/mandb
/usr/local/bin/manpath
/usr/local/bin/whatis
/usr/local/sbin/accessdb
```

The complete list is `stage/<arch>/.manifest/man-db` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa man-db
ls dists/8.0/*/ | grep '^man-db-'
```

This writes `man-db-<ver>-r0.tar.gz` (run time) and `man-db-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add man-db
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/man-db-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/man-db-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | man-db 2.13.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/man-db-2.13.1-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | man-db 2.13.1-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/man-db-2.13.1-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2.13.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add man-db=<version>`), sources in `ported/man-db/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates man-db`; port one with `scripts/port-update man-db`.

<details><summary>recipe.sh</summary>

```bash
# man-db, the man page reader. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=2.13.1
TARBALL=https://download.savannah.gnu.org/releases/man-db/man-db-2.13.1.tar.xz
SHA256=8afebb6f7eb6bb8542929458841f5c7e6f240e30c86358c1fbcefbea076c87d9
# Also linked at run time (found by scripts/gen-dependency-index): libiconv zlib
DEPS="libpipeline gdbm groff libiconv zlib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --disable-setuid --without-libseccomp --disable-cache-owner --with-systemdtmpfilesdir=no --with-systemdsystemunitdir=no )
# The "manual" directory typesets man-db's user guide with groff's -me macros
# on the build host, where they are often not installed (Debian's groff-base
# lacks them). The guide is not needed to run man; leave that directory out of
# the generated top-level Makefile. (Not with SUBDIRS=... on the make command
# line: that reaches every sub-make too, and gl/lib then looks for "docs".)
_no_manual() { sed -i '/^SUBDIRS = /s/ manual / /' "$B/Makefile"; }
do_build()   { _no_manual && make -C "$B" -j"$(nproc)"; }
do_install() { _no_manual && make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
