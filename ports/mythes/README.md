# mythes 1.2.4

> MyThes thesaurus library (OpenOffice/LibreOffice thesaurus format)

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://downloads.sourceforge.net/hunspell/mythes-1.2.4.tar.gz |
| SHA-256 | `1e81f395d8c851c3e4e75b568e20fa2fa549354e75ab397f9de4b0e0790a305f` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`hunspell`](../hunspell/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_mythes`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `mythes` (programs, shared libraries, run-time data) and `mythes-dev` (headers, pkg-config/CMake files, static libraries; needs `mythes`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`mythes`](https://openqnx.github.io/oss/package/x86_64/mythes.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mythes.html)) · [`mythes-dev`](https://openqnx.github.io/oss/package/x86_64/mythes-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/mythes-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add mythes` (dependencies come along); one version: `apk add mythes=<version>` | `apk del mythes` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install mythes` (dependencies first) | `oss-get remove mythes` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`mythes-dev` the same way, by its own name (`apk add mythes-dev`, `oss-get install mythes-dev`, by hand with its own file and list `mythes-dev.list`); remove it before `mythes`, which it needs.

```sh
# on the QNX target, as root
apk add mythes mythes-dev
apk add qnx-test-mythes && /usr/local/bin/qnx-port-tests/test_mythes    # its on-target test, from tests-8.0-<arch>-m-z
apk del mythes mythes-dev qnx-test-mythes
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/mythes.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/mythes.list
rm var/lib/manual/mythes.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild mythes` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild hunspell
```

<details><summary>Full build chain (2)</summary>

[`hunspell`](../hunspell/) [`libiconv`](../libiconv/)
</details>

### 2. Get the upstream source

```bash
URL=https://downloads.sourceforge.net/hunspell/mythes-1.2.4.tar.gz
mkdir -p dist src/mythes
curl -fL -o dist/$(basename $URL) $URL
echo "1e81f395d8c851c3e4e75b568e20fa2fa549354e75ab397f9de4b0e0790a305f  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/mythes --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/mythes; B=build/$QNX_ARCH/mythes; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i '1s|^#!/usr/bin/perl|#!/usr/local/bin/perl|' "$STAGE$PREFIX/bin/th_gen_idx.pl"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- MyThes: the thesaurus library of OpenOffice/LibreOffice (libmythes-1.2): looks a word up in a thesaurus data file (.dat) through its index (.idx) and returns its meanings and synonyms; th_gen_idx.pl writes the index of a .dat file. No thesaurus data is included (per-language, e.g. th_en_US_v2.dat). Version as in QNX's own package list (the latest release).
- hunspell: the example program (not installed) stems words with it; the library itself does not use it.
- th_gen_idx.pl names /usr/bin/perl; perl is the ported one in the prefix.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH mythes`.

### 5. What it installs

6 files under `/usr/local`: 2 shared-library entries, 1 programs, 1 headers.

```
/usr/local/lib/libmythes-1.2.so
/usr/local/lib/libmythes-1.2.so.0
/usr/local/bin/th_gen_idx.pl
```

The complete list is `stage/<arch>/.manifest/mythes` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa mythes
ls dists/8.0/*/ | grep '^mythes-'
```

This writes `mythes-<ver>-r0.tar.gz` (run time) and `mythes-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add mythes
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/mythes-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/mythes-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_mythes`](../../tests/src/) links mythes the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-mythes`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-mythes
/usr/local/bin/qnx-port-tests/test_mythes
```

```
== mythes ==
  ok   ...
PASS mythes
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_mythes` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_mythes`](../../tests/src/) -- a program linked against mythes the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - encoding UTF-8
   - Lookup(unknownword) finds nothing
   - Lookup(ha), a prefix of an entry, finds nothing
   - th_gen_idx.pl writes the same index

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | mythes 1.2.4-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/mythes-1.2.4-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | mythes 1.2.4-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/mythes-1.2.4-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.2.4**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add mythes=<version>`), sources in `ported/mythes/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates mythes`; port one with `scripts/port-update mythes`.

<details><summary>recipe.sh</summary>

```bash
# MyThes: the thesaurus library of OpenOffice/LibreOffice (libmythes-1.2):
# looks a word up in a thesaurus data file (.dat) through its index (.idx)
# and returns its meanings and synonyms; th_gen_idx.pl writes the index of a
# .dat file. No thesaurus data is included (per-language, e.g.
# th_en_US_v2.dat). Version as in QNX's own package list (the latest release).
VERSION=1.2.4
TARBALL=https://downloads.sourceforge.net/hunspell/mythes-1.2.4.tar.gz
SHA256=1e81f395d8c851c3e4e75b568e20fa2fa549354e75ab397f9de4b0e0790a305f
BUILDSYS=autotools
# hunspell: the example program (not installed) stems words with it; the
# library itself does not use it.
DEPS="hunspell"
# th_gen_idx.pl names /usr/bin/perl; perl is the ported one in the prefix.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i '1s|^#!/usr/bin/perl|#!/usr/local/bin/perl|' "$STAGE$PREFIX/bin/th_gen_idx.pl"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
