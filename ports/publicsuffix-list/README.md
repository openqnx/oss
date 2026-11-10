# publicsuffix-list 20261031

> Public Suffix List: the cross-vendor database of public domain suffixes

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/publicsuffix/list.git |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_publicsuffix_list`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `publicsuffix-list`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`publicsuffix-list`](https://openqnx.github.io/oss/package/x86_64/publicsuffix-list.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/publicsuffix-list.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add publicsuffix-list` (dependencies come along); one version: `apk add publicsuffix-list=<version>` | `apk del publicsuffix-list` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install publicsuffix-list` (dependencies first) | `oss-get remove publicsuffix-list` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-publicsuffix-list && /usr/local/bin/qnx-port-tests/test_publicsuffix_list    # its on-target test, from tests-8.0-<arch>-m-z
apk del publicsuffix-list qnx-test-publicsuffix-list
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/publicsuffix-list.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/publicsuffix-list.list
rm var/lib/manual/publicsuffix-list.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild publicsuffix-list` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/publicsuffix/list.git src/publicsuffix-list
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/publicsuffix-list; B=build/$QNX_ARCH/publicsuffix-list; mkdir -p $B
```

```bash
do_build() { :; }
```

```bash
do_install() { local d="$STAGE$PREFIX/share/publicsuffix"
  install -Dm644 "$S/public_suffix_list.dat" "$d/public_suffix_list.dat" &&
  install -m644 "$S/tests/test_psl.txt" "$d/test_psl.txt" &&
  ln -sf public_suffix_list.dat "$d/effective_tld_names.dat"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The Public Suffix List: the cross-vendor database of domain suffixes under which anyone can register names (com, co.uk, github.io), used to decide cookie and certificate scopes. Installs share/publicsuffix/ public_suffix_list.dat (with the effective_tld_names.dat name older consumers use) and upstream's test_psl.txt checks. Data, the same for every architecture; libpsl reads it with psl_load_file() or psl --load-psl-file. Upstream has no releases: VERSION is the commit's date.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH publicsuffix-list`.

### 5. What it installs

3 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/publicsuffix-list` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa publicsuffix-list
ls dists/8.0/*/ | grep '^publicsuffix-list-'
```

This writes `publicsuffix-list-<ver>-r0.tar.gz` (run time) and `publicsuffix-list-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add publicsuffix-list
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/publicsuffix-list-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/publicsuffix-list-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_publicsuffix_list`](../../tests/src/) links publicsuffix-list the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-publicsuffix-list`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-publicsuffix-list
/usr/local/bin/qnx-port-tests/test_publicsuffix_list
```

```
== publicsuffix-list ==
  ok   ...
PASS publicsuffix-list
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_publicsuffix_list` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_publicsuffix_list`](../../tests/src/) -- a program linked against publicsuffix-list the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - psl_load_file(public_suffix_list.dat)
   - more than 9000 rules
   - co.uk and github.io are public suffixes, example.co.uk is not
   - test_psl.txt is installed
   - every test_psl.txt check (registrable domains) passes
   - effective_tld_names.dat loads the same list
   - psl --load-psl-file: bbc.co.uk, foo.github.io

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | publicsuffix-list 20261031-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/publicsuffix-list-20261031-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | publicsuffix-list 20261031-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/publicsuffix-list-20261031-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **20261031**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add publicsuffix-list=<version>`), sources in `ported/publicsuffix-list/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates publicsuffix-list`; port one with `scripts/port-update publicsuffix-list`.

<details><summary>recipe.sh</summary>

```bash
# The Public Suffix List: the cross-vendor database of domain suffixes under
# which anyone can register names (com, co.uk, github.io), used to decide
# cookie and certificate scopes. Installs share/publicsuffix/
# public_suffix_list.dat (with the effective_tld_names.dat name older
# consumers use) and upstream's test_psl.txt checks. Data, the same for
# every architecture; libpsl reads it with psl_load_file() or psl
# --load-psl-file. Upstream has no releases: VERSION is the commit's date.
VERSION=20261031; GIT=https://github.com/publicsuffix/list.git; COMMIT=73f8cc701c084fe07d888ec552b2f0b53281a674
BUILDSYS=custom
do_build() { :; }
do_install() { local d="$STAGE$PREFIX/share/publicsuffix"
  install -Dm644 "$S/public_suffix_list.dat" "$d/public_suffix_list.dat" &&
  install -m644 "$S/tests/test_psl.txt" "$d/test_psl.txt" &&
  ln -sf public_suffix_list.dat "$d/effective_tld_names.dat"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
