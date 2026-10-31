# perl-error 0.17030

> Error.pm, try/catch style exception handling for Perl

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://cpan.metacpan.org/authors/id/S/SH/SHLOMIF/Error-0.17030.tar.gz |
| SHA-256 | `34d382276c0fb0d6b38355b94c96a30b12d834d5662eb53f088ee25e3e712924` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | [`perl`](../perl/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_perl_error`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add perl-error
apk add perl-error-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-perl-error && /usr/local/bin/qnx-port-tests/test_perl_error    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install perl-error`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild perl-error` performs exactly the same steps (2–5) in one command.

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
URL=https://cpan.metacpan.org/authors/id/S/SH/SHLOMIF/Error-0.17030.tar.gz
mkdir -p dist src/perl-error
curl -fL -o dist/$(basename $URL) $URL
echo "34d382276c0fb0d6b38355b94c96a30b12d834d5662eb53f088ee25e3e712924  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/perl-error --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/perl-error; B=build/$QNX_ARCH/perl-error; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/lib/perl5/site_perl/Error" && install -m644 "$S/lib/Error.pm" "$STAGE$PREFIX/lib/perl5/site_perl/" && \
  cp -a "$S/lib/Error/." "$STAGE$PREFIX/lib/perl5/site_perl/Error/"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Error.pm (try/catch exception objects for Perl; used by git's Perl scripts). Pure Perl, so the module files are installed as released into the target perl's library. Version as in the QNX Everywhere 8.0.4r1 image.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH perl-error`.

### 5. What it installs

2 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/perl-error` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa perl-error
ls dists/8.0/*/ | grep '^perl-error-'
```

This writes `perl-error-<ver>-r0.tar.gz` (run time) and `perl-error-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add perl-error
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/perl-error-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/perl-error-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_perl_error`](../../tests/src/) links perl-error the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-perl-error`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-perl-error
/usr/local/bin/qnx-port-tests/test_perl_error
```

```
== perl-error ==
  ok   ...
PASS perl-error
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_perl_error` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_perl_error`](../../tests/src/) -- a program linked against perl-error the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl-error 0.17030-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/perl-error-0.17030-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl-error 0.17030-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/perl-error-0.17030-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **0.17030**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add perl-error=<version>`), sources in `ported/perl-error/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates perl-error`; port one with `scripts/port-update perl-error`.

<details><summary>recipe.sh</summary>

```bash
# Error.pm (try/catch exception objects for Perl; used by git's Perl
# scripts). Pure Perl, so the module files are installed as released into the
# target perl's library. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=0.17030
TARBALL=https://cpan.metacpan.org/authors/id/S/SH/SHLOMIF/Error-0.17030.tar.gz
SHA256=34d382276c0fb0d6b38355b94c96a30b12d834d5662eb53f088ee25e3e712924
DEPS="perl"
BUILDSYS=custom
do_build() { :; }
do_install() { install -d "$STAGE$PREFIX/lib/perl5/site_perl/Error" && install -m644 "$S/lib/Error.pm" "$STAGE$PREFIX/lib/perl5/site_perl/" && \
  cp -a "$S/lib/Error/." "$STAGE$PREFIX/lib/perl5/site_perl/Error/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
