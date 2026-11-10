# tzdata 2026f

> IANA time zone database (zoneinfo TZif files) with zic and zdump

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://data.iana.org/time-zones/releases/tzdb-2026f.tar.lz |
| SHA-256 | `357829e3a16a78224d23f1ae2327704c1da1fd35f6769a7aac0682755fb9027a` |
| Build system | custom (see recipe) |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_tzdata_tools`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `tzdata`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`tzdata`](https://openqnx.github.io/oss/package/x86_64/tzdata.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/tzdata.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add tzdata` (dependencies come along); one version: `apk add tzdata=<version>` | `apk del tzdata` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install tzdata` (dependencies first) | `oss-get remove tzdata` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-tzdata-tools && /usr/local/bin/qnx-port-tests/test_tzdata_tools    # its on-target test, from tests-8.0-<arch>-m-z
apk del tzdata qnx-test-tzdata-tools
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/tzdata.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/tzdata.list
rm var/lib/manual/tzdata.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild tzdata` performs exactly the same steps (2–5) in one command.

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
URL=https://data.iana.org/time-zones/releases/tzdb-2026f.tar.lz
mkdir -p dist src/tzdata
curl -fL -o dist/$(basename $URL) $URL
echo "357829e3a16a78224d23f1ae2327704c1da1fd35f6769a7aac0682755fb9027a  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/tzdata --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/tzdata; B=build/$QNX_ARCH/tzdata; mkdir -p $B
```

```bash
do_configure() { :; }
```

```bash
do_build() {
  # tzcode's Makefile builds in the source tree: one copy for the build
  # machine's zic (it writes the data, so it has to run here) and one for the
  # target's programs.
  rm -rf "$B/host" "$B/target"; mkdir -p "$B"
  cp -a "$S" "$B/host" && cp -a "$S" "$B/target" || return 1
  make -C "$B/host" CC=gcc CFLAGS=-O2 zic || return 1
  # TZDIR: where the target's zdump and zic look for zone names by default.
  # HAVE_GETTEXT=0: the staged libintl.h (gettext) would otherwise switch on
  # message translation and a dependency on libintl for two small tools.
  # HAVE_STRFTIME_L=0: QNX has no locale_t / strftime_l; HAVE_GETRESUID=0:
  # nor getresuid/getresgid (zdump carries tzcode's own localtime.c).
  make -C "$B/target" CC="qcc -V$QCC_TARGET" AR="nto${QNX_CPU}-ar" CFLAGS="-O2 -D_QNX_SOURCE -DHAVE_GETTEXT=0 -DHAVE_STRFTIME_L=0 -DHAVE_GETRESUID=0" \
       TZDIR="$PREFIX/share/zoneinfo" zic zdump
}
```

```bash
do_install() {
  local z="$STAGE$PREFIX/share/zoneinfo"
  mkdir -p "$z" "$STAGE$PREFIX/bin"
  # -b fat: the transitions are written out in full up to 2037 rather than
  # left to the POSIX TZ footer, as Alpine and QNX build it -- readers that
  # only take the transition table (older libcs) get the right answers too.
  ( cd "$S" && "$B/host/zic" -b fat -d "$z" $TZ_FILES ) || return 1
  # posixrules: the rules a POSIX TZ string without a rule part ("EST5EDT")
  # takes, as tzcode-derived libcs (QNX's among them) look for it.
  ln -sf America/New_York "$z/posixrules"
  install -m644 "$S"/zone.tab "$S"/zone1970.tab "$S"/zonenow.tab "$S"/iso3166.tab \
                "$S"/tzdata.zi "$S"/leap-seconds.list "$z/"
  echo "$VERSION" > "$z/+VERSION"
  install -m755 "$B/target/zic" "$B/target/zdump" "$STAGE$PREFIX/bin/"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- tzdata: the IANA time zone database compiled into TZif files under share/zoneinfo (Area/City names), with zone.tab, zone1970.tab, iso3166.tab, tzdata.zi and leap-seconds.list, plus the target's zic and zdump. QNX's libc reads a TZif file when TZ names one by path (TZ=/usr/local/share/ zoneinfo/Europe/Berlin, or TZ=:<path>): the whole history of a zone, not only the rules of a POSIX TZ string; Python's zoneinfo, ICU-less programs and anything that opens /usr/share/zoneinfo-style files use it the same way. QNX's own package list has 2025b; this is the IANA release current when it was ported (the data changes several times a year).
- The zones zic compiles: the main data files (not backzone, which is the pre-1970 detail Alpine and QNX leave out too).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH tzdata`.

### 5. What it installs

608 files under `/usr/local`, 2 programs.

```
/usr/local/bin/zdump
/usr/local/bin/zic
```

The complete list is `stage/<arch>/.manifest/tzdata` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa tzdata
ls dists/8.0/*/ | grep '^tzdata-'
```

This writes `tzdata-<ver>-r0.tar.gz` (run time) and `tzdata-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add tzdata
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/tzdata-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/tzdata-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_tzdata_tools`](../../tests/src/) links tzdata the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-tzdata-tools`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-tzdata-tools
/usr/local/bin/qnx-port-tests/test_tzdata_tools
```

```
== tzdata ==
  ok   ...
PASS tzdata
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_tzdata_tools` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_tzdata_tools`](../../tests/src/) -- a program linked against tzdata the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | tzdata 2026f-r0 | ✅ PASS | 2026-11-05 | [log](../../tests/logs/aarch64le/tzdata-2026f-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | tzdata 2026f-r0 | ✅ PASS | 2026-11-04 | [log](../../tests/logs/x86_64/tzdata-2026f-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **2026f**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add tzdata=<version>`), sources in `ported/tzdata/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates tzdata`; port one with `scripts/port-update tzdata`.

<details><summary>recipe.sh</summary>

```bash
# tzdata: the IANA time zone database compiled into TZif files under
# share/zoneinfo (Area/City names), with zone.tab, zone1970.tab, iso3166.tab,
# tzdata.zi and leap-seconds.list, plus the target's zic and zdump. QNX's
# libc reads a TZif file when TZ names one by path (TZ=/usr/local/share/
# zoneinfo/Europe/Berlin, or TZ=:<path>): the whole history of a zone, not
# only the rules of a POSIX TZ string; Python's zoneinfo, ICU-less programs and
# anything that opens /usr/share/zoneinfo-style files use it the same way.
# QNX's own package list has 2025b; this is the IANA release current when it
# was ported (the data changes several times a year).
VERSION=2026f
TARBALL=https://data.iana.org/time-zones/releases/tzdb-2026f.tar.lz
SHA256=357829e3a16a78224d23f1ae2327704c1da1fd35f6769a7aac0682755fb9027a
BUILDSYS=custom
DEPS=""
# The zones zic compiles: the main data files (not backzone, which is the
# pre-1970 detail Alpine and QNX leave out too).
TZ_FILES="africa antarctica asia australasia europe northamerica southamerica etcetera backward factory"
do_configure() { :; }
do_build() {
  # tzcode's Makefile builds in the source tree: one copy for the build
  # machine's zic (it writes the data, so it has to run here) and one for the
  # target's programs.
  rm -rf "$B/host" "$B/target"; mkdir -p "$B"
  cp -a "$S" "$B/host" && cp -a "$S" "$B/target" || return 1
  make -C "$B/host" CC=gcc CFLAGS=-O2 zic || return 1
  # TZDIR: where the target's zdump and zic look for zone names by default.
  # HAVE_GETTEXT=0: the staged libintl.h (gettext) would otherwise switch on
  # message translation and a dependency on libintl for two small tools.
  # HAVE_STRFTIME_L=0: QNX has no locale_t / strftime_l; HAVE_GETRESUID=0:
  # nor getresuid/getresgid (zdump carries tzcode's own localtime.c).
  make -C "$B/target" CC="qcc -V$QCC_TARGET" AR="nto${QNX_CPU}-ar" CFLAGS="-O2 -D_QNX_SOURCE -DHAVE_GETTEXT=0 -DHAVE_STRFTIME_L=0 -DHAVE_GETRESUID=0" \
       TZDIR="$PREFIX/share/zoneinfo" zic zdump
}
do_install() {
  local z="$STAGE$PREFIX/share/zoneinfo"
  mkdir -p "$z" "$STAGE$PREFIX/bin"
  # -b fat: the transitions are written out in full up to 2037 rather than
  # left to the POSIX TZ footer, as Alpine and QNX build it -- readers that
  # only take the transition table (older libcs) get the right answers too.
  ( cd "$S" && "$B/host/zic" -b fat -d "$z" $TZ_FILES ) || return 1
  # posixrules: the rules a POSIX TZ string without a rule part ("EST5EDT")
  # takes, as tzcode-derived libcs (QNX's among them) look for it.
  ln -sf America/New_York "$z/posixrules"
  install -m644 "$S"/zone.tab "$S"/zone1970.tab "$S"/zonenow.tab "$S"/iso3166.tab \
                "$S"/tzdata.zi "$S"/leap-seconds.list "$z/"
  echo "$VERSION" > "$z/+VERSION"
  install -m755 "$B/target/zic" "$B/target/zdump" "$STAGE$PREFIX/bin/"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
