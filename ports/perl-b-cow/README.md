# perl-b-cow 0.007

> B::COW, Perl's copy-on-write string internals (is a scalar COW, its refcount)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://cpan.metacpan.org/authors/id/A/AT/ATOOMIC/B-COW-0.007.tar.gz |
| SHA-256 | `1290daf227e8b09889a31cf182e29106f1cf9f1a4e9bf7752f9de92ed1158b44` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`perl`](../perl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_perl_xs_misc`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `perl-b-cow` (programs, shared libraries, run-time data) and `perl-b-cow-dev` (headers, pkg-config/CMake files, static libraries; needs `perl-b-cow`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`perl-b-cow`](https://openqnx.github.io/oss/package/x86_64/perl-b-cow.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-b-cow.html)) · [`perl-b-cow-dev`](https://openqnx.github.io/oss/package/x86_64/perl-b-cow-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-b-cow-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add perl-b-cow` (dependencies come along); one version: `apk add perl-b-cow=<version>` | `apk del perl-b-cow` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install perl-b-cow` (dependencies first) | `oss-get remove perl-b-cow` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`perl-b-cow-dev` the same way, by its own name (`apk add perl-b-cow-dev`, `oss-get install perl-b-cow-dev`, by hand with its own file and list `perl-b-cow-dev.list`); remove it before `perl-b-cow`, which it needs.

```sh
# on the QNX target, as root
apk add perl-b-cow perl-b-cow-dev
apk add qnx-test-perl-xs-misc && /usr/local/bin/qnx-port-tests/test_perl_xs_misc    # its on-target test, from tests-8.0-<arch>-m-z
apk del perl-b-cow perl-b-cow-dev qnx-test-perl-xs-misc
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/perl-b-cow.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/perl-b-cow.list
rm var/lib/manual/perl-b-cow.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild perl-b-cow` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (2 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild perl
```

<details><summary>Full build chain (2)</summary>

[`libxcrypt`](../libxcrypt/) [`perl`](../perl/)
</details>

### 2. Get the upstream source

```bash
URL=https://cpan.metacpan.org/authors/id/A/AT/ATOOMIC/B-COW-0.007.tar.gz
mkdir -p dist src/perl-b-cow
curl -fL -o dist/$(basename $URL) $URL
echo "1290daf227e8b09889a31cf182e29106f1cf9f1a4e9bf7752f9de92ed1158b44  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/perl-b-cow --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/perl-b-cow; B=build/$QNX_ARCH/perl-b-cow; mkdir -p $B
```

```bash
do_configure() { qnx_perl_xs_configure; }
```

```bash
do_build() { qnx_perl_xs_make; }
```

```bash
do_install() { qnx_perl_xs_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- perl-b-cow: B::COW, Perl's copy-on-write string internals (is a scalar COW, its refcount).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH perl-b-cow`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/perl-b-cow` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa perl-b-cow
ls dists/8.0/*/ | grep '^perl-b-cow-'
```

This writes `perl-b-cow-<ver>-r0.tar.gz` (run time) and `perl-b-cow-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add perl-b-cow
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/perl-b-cow-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/perl-b-cow-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_perl_xs_misc`](../../tests/src/) links perl-b-cow the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-perl-xs-misc`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-perl-xs-misc
/usr/local/bin/qnx-port-tests/test_perl_xs_misc
```

```
== perl-b-cow ==
  ok   ...
PASS perl-b-cow
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_perl_xs_misc` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_perl_xs_misc`](../../tests/src/) -- a program linked against perl-b-cow the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl-b-cow 0.007-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/perl-b-cow-0.007-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl-clone 0.51-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/perl-clone-0.51-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl-fcgi 0.82-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/perl-fcgi-0.82-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl-b-cow 0.007-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/perl-b-cow-0.007-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl-clone 0.51-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/perl-clone-0.51-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl-fcgi 0.82-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/x86_64/perl-fcgi-0.82-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.007**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add perl-b-cow=<version>`), sources in `ported/perl-b-cow/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates perl-b-cow`; port one with `scripts/port-update perl-b-cow`.

<details><summary>recipe.sh</summary>

```bash
# perl-b-cow: B::COW, Perl's copy-on-write string internals (is a scalar COW, its refcount).
VERSION=0.007
TARBALL=https://cpan.metacpan.org/authors/id/A/AT/ATOOMIC/B-COW-0.007.tar.gz
SHA256=1290daf227e8b09889a31cf182e29106f1cf9f1a4e9bf7752f9de92ed1158b44
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { qnx_perl_xs_configure; }
do_build() { qnx_perl_xs_make; }
do_install() { qnx_perl_xs_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
