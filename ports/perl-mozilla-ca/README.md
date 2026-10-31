# perl-mozilla-ca 20261020

> Mozilla's CA certificate bundle for Perl (Mozilla::CA)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://cpan.metacpan.org/authors/id/L/LW/LWP/Mozilla-CA-20261020.tar.gz |
| SHA-256 | `1aeed97a463389e582e085e82de03ae0bffae74d46cf8ee4c511341e5508c21d` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`perl`](../perl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_perl_mozilla_ca`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `perl-mozilla-ca`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`perl-mozilla-ca`](https://openqnx.github.io/oss/package/x86_64/perl-mozilla-ca.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-mozilla-ca.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add perl-mozilla-ca` (dependencies come along); one version: `apk add perl-mozilla-ca=<version>` | `apk del perl-mozilla-ca` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install perl-mozilla-ca` (dependencies first) | `oss-get remove perl-mozilla-ca` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-perl-mozilla-ca && /usr/local/bin/qnx-port-tests/test_perl_mozilla_ca    # its on-target test
apk del perl-mozilla-ca qnx-test-perl-mozilla-ca
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/perl-mozilla-ca.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/perl-mozilla-ca.list
rm var/lib/manual/perl-mozilla-ca.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild perl-mozilla-ca` performs exactly the same steps (2–5) in one command.

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
URL=https://cpan.metacpan.org/authors/id/L/LW/LWP/Mozilla-CA-20261020.tar.gz
mkdir -p dist src/perl-mozilla-ca
curl -fL -o dist/$(basename $URL) $URL
echo "1aeed97a463389e582e085e82de03ae0bffae74d46cf8ee4c511341e5508c21d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/perl-mozilla-ca --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/perl-mozilla-ca; B=build/$QNX_ARCH/perl-mozilla-ca; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() {
  local pv; pv=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/perl/recipe.sh")
  local d="$STAGE$PREFIX/lib/perl5/site_perl/$pv"; mkdir -p "$d"
  cp -a "$S"/*/lib/Mozilla "$d/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- perl-mozilla-ca: Mozilla::CA, Mozilla's CA certificate bundle for Perl (LWP and IO::Socket::SSL verify HTTPS servers with it). Pure Perl and a PEM file: installed into Perl's site library.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH perl-mozilla-ca`.

### 5. What it installs

2 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/perl-mozilla-ca` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa perl-mozilla-ca
ls dists/8.0/*/ | grep '^perl-mozilla-ca-'
```

This writes `perl-mozilla-ca-<ver>-r0.tar.gz` (run time) and `perl-mozilla-ca-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add perl-mozilla-ca
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/perl-mozilla-ca-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/perl-mozilla-ca-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_perl_mozilla_ca`](../../tests/src/) links perl-mozilla-ca the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-perl-mozilla-ca`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-perl-mozilla-ca
/usr/local/bin/qnx-port-tests/test_perl_mozilla_ca
```

```
== perl-mozilla-ca ==
  ok   ...
PASS perl-mozilla-ca
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_perl_mozilla_ca` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_perl_mozilla_ca`](../../tests/src/) -- a program linked against perl-mozilla-ca the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl-mozilla-ca 20261020-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/perl-mozilla-ca-20261020-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl-mozilla-ca 20261020-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/perl-mozilla-ca-20261020-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20261020**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add perl-mozilla-ca=<version>`), sources in `ported/perl-mozilla-ca/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates perl-mozilla-ca`; port one with `scripts/port-update perl-mozilla-ca`.

<details><summary>recipe.sh</summary>

```bash
# perl-mozilla-ca: Mozilla::CA, Mozilla's CA certificate bundle for Perl
# (LWP and IO::Socket::SSL verify HTTPS servers with it). Pure Perl and a
# PEM file: installed into Perl's site library.
VERSION=20261020
TARBALL=https://cpan.metacpan.org/authors/id/L/LW/LWP/Mozilla-CA-20261020.tar.gz
SHA256=1aeed97a463389e582e085e82de03ae0bffae74d46cf8ee4c511341e5508c21d
DEPS="perl"
RUNTIME_DEPS="perl"
BUILDSYS=custom
source "$WS/scripts/lib/fonts.sh"
do_fetch() { qnx_data_fetch; }
do_configure() { :; }
do_build() { :; }
do_install() {
  local pv; pv=$(sed -n 's/^VERSION=\([^;]*\).*/\1/p' "$WS/ports/perl/recipe.sh")
  local d="$STAGE$PREFIX/lib/perl5/site_perl/$pv"; mkdir -p "$d"
  cp -a "$S"/*/lib/Mozilla "$d/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
