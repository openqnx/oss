# perl-libwww 6.83

> libwww-perl (LWP), the World-Wide Web library for Perl: LWP::UserAgent and the lwp-* tools

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://cpan.metacpan.org/authors/id/O/OA/OALDERS/libwww-perl-6.83.tar.gz |
| SHA-256 | `e75f0fa9d3c6f0daf5a5a72fa9f8b1c9c0d23e3a84a8522ccb4f835232b95505` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`perl`](../perl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_perl_http`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `perl-libwww`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`perl-libwww`](https://openqnx.github.io/oss/package/x86_64/perl-libwww.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-libwww.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add perl-libwww` (dependencies come along); one version: `apk add perl-libwww=<version>` | `apk del perl-libwww` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install perl-libwww` (dependencies first) | `oss-get remove perl-libwww` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-perl-http && /usr/local/bin/qnx-port-tests/test_perl_http    # its on-target test, from tests-8.0-<arch>-m-z
apk del perl-libwww qnx-test-perl-http
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/perl-libwww.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/perl-libwww.list
rm var/lib/manual/perl-libwww.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild perl-libwww` performs exactly the same steps (2–5) in one command.

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
URL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/libwww-perl-6.83.tar.gz
mkdir -p dist src/perl-libwww
curl -fL -o dist/$(basename $URL) $URL
echo "e75f0fa9d3c6f0daf5a5a72fa9f8b1c9c0d23e3a84a8522ccb4f835232b95505  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/perl-libwww --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/perl-libwww; B=build/$QNX_ARCH/perl-libwww; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() { :; }
```

```bash
do_install() { qnx_perl_pure_install bin/lwp-download bin/lwp-dump bin/lwp-mirror bin/lwp-request; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- perl-libwww: libwww-perl (LWP), the World-Wide Web library for Perl: LWP::UserAgent and the lwp-* tools. Pure Perl: lib/ as released into the target perl's site library (scripts/lib/perl-module.sh).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH perl-libwww`.

### 5. What it installs

27 files under `/usr/local`, 4 programs.

```
/usr/local/bin/lwp-download
/usr/local/bin/lwp-dump
/usr/local/bin/lwp-mirror
/usr/local/bin/lwp-request
```

The complete list is `stage/<arch>/.manifest/perl-libwww` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa perl-libwww
ls dists/8.0/*/ | grep '^perl-libwww-'
```

This writes `perl-libwww-<ver>-r0.tar.gz` (run time) and `perl-libwww-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add perl-libwww
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/perl-libwww-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/perl-libwww-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_perl_http`](../../tests/src/) links perl-libwww the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-perl-http`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-perl-http
/usr/local/bin/qnx-port-tests/test_perl_http
```

```
== perl-libwww ==
  ok   ...
PASS perl-libwww
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_perl_http` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_perl_http`](../../tests/src/) -- a program linked against perl-libwww the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> perl-libwww`.

## Versions

This recipe builds **6.83**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add perl-libwww=<version>`), sources in `ported/perl-libwww/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates perl-libwww`; port one with `scripts/port-update perl-libwww`.

<details><summary>recipe.sh</summary>

```bash
# perl-libwww: libwww-perl (LWP), the World-Wide Web library for Perl: LWP::UserAgent and the lwp-* tools.
# Pure Perl: lib/ as released into the target perl's site library
# (scripts/lib/perl-module.sh).
VERSION=6.83
TARBALL=https://cpan.metacpan.org/authors/id/O/OA/OALDERS/libwww-perl-6.83.tar.gz
SHA256=e75f0fa9d3c6f0daf5a5a72fa9f8b1c9c0d23e3a84a8522ccb4f835232b95505
DEPS="perl"
RUNTIME_DEPS="perl perl-encode-locale perl-file-listing perl-html-parser perl-http-cookies perl-http-date perl-http-message perl-http-negotiate perl-lwp-mediatypes perl-net-http perl-try-tiny perl-uri perl-www-robotrules"
BUILDSYS=custom
source "$WS/scripts/lib/perl-module.sh"
do_configure() { :; }
do_build() { :; }
do_install() { qnx_perl_pure_install bin/lwp-download bin/lwp-dump bin/lwp-mirror bin/lwp-request; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
