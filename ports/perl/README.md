# perl 5.40.2

> Perl 5 programming language interpreter

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.cpan.org/src/5.0/perl-5.40.2.tar.gz |
| SHA-256 | `10d4647cfbb543a7f9ae3e5f6851ec49305232ea7621aed24c7cfbb0bef4b70d` |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`libxcrypt`](../libxcrypt/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `perl` (programs, shared libraries, run-time data) and `perl-dev` (headers, pkg-config/CMake files, static libraries; needs `perl`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`perl`](https://openqnx.github.io/oss/package/x86_64/perl.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl.html)) · [`perl-dev`](https://openqnx.github.io/oss/package/x86_64/perl-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/perl-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add perl` (dependencies come along); one version: `apk add perl=<version>` | `apk del perl` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install perl` (dependencies first) | `oss-get remove perl` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`perl-dev` the same way, by its own name (`apk add perl-dev`, `oss-get install perl-dev`, by hand with its own file and list `perl-dev.list`); remove it before `perl`, which it needs.

```sh
# on the QNX target, as root
apk add perl perl-dev
apk del perl perl-dev
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/perl.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/perl.list
rm var/lib/manual/perl.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild perl` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (1 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild libxcrypt
```

### 2. Get the upstream source

```bash
URL=https://www.cpan.org/src/5.0/perl-5.40.2.tar.gz
mkdir -p dist src/perl
curl -fL -o dist/$(basename $URL) $URL
echo "10d4647cfbb543a7f9ae3e5f6851ec49305232ea7621aed24c7cfbb0bef4b70d  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/perl --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/perl < ports/perl/patches/0001-errno-target-osname.patch
patch -p1 -d src/perl < ports/perl/patches/0002-qnx-atfork-child-reinit.patch
```

**[`0001-errno-target-osname.patch`](patches/0001-errno-target-osname.patch)** — Errno_pm.PL: decide by the target OS, not the build host's.

> When cross compiling, Errno_pm.PL runs under the build host's miniperl,
> where $^O is "linux". It then takes its Linux path, preprocesses the host's
> glibc <errno.h> with the QNX compiler ("features.h: No such file") and finds
> no error definitions. $Config{osname} is the target's ("nto").

Touches: `ext/Errno/Errno_pm.PL` · Upstream status: QNX-specific

**[`0002-qnx-atfork-child-reinit.patch`](patches/0002-qnx-atfork-child-reinit.patch)** — QNX: re-initialise the atfork mutexes in the child instead of unlocking them

> On QNX the forked child's thread does not own the mutexes the parent's
> thread locked in Perl_atfork_lock() (the pthread_atfork prepare handler), so
> Perl_atfork_unlock() in the child failed with EPERM and MUTEX_UNLOCK panicked:
> every fork (backticks, system, open "-|") from a threaded perl died, and
> autoconf/automake hung. The child now re-initialises PL_perlio_mutex,
> PL_malloc_mutex and PL_op_mutex; the parent still unlocks them.
> No symbol or structure changes (the pid is a file-static).

Touches: `util.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/perl; B=build/$QNX_ARCH/perl; mkdir -p $B
```

```bash
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" -Dcc="qcc -V$QCC_TARGET" -Dld="qcc -V$QCC_TARGET" \
    -Dar="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" -Dccflags="-D_QNX_SOURCE" -Duseshrplib -Dusethreads \
    -Dusrinc="$QNX_TARGET_SDP/usr/include" -Dincpth="$QNX_TARGET_SDP/usr/include" \
    -Dcpp="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E -P" -Dgccversion="$("$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" -dumpversion)" \
    -Dcpprun="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" -Dcppstdin="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    -Dlibpth="$QNX_TARGET/$QNX_ARCH/lib $QNX_TARGET/$QNX_ARCH/usr/lib"
}
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
- Perl cannot cross compile with its own Configure (it runs probes on the target). perl-cross replaces Configure with a cross-capable one: its tree is unpacked over the perl source (do_fetch). perl-cross knows QNX (osname "nto", cnf/hints/nto) since 1.6.2 -- the change qnx-ports carried for 1.6.1 went upstream -- so only the perl patch in patches/ remains. This is the perl version the QNX Everywhere 8.0.4r1 image ships; it also covers qnx-ports' perl5 port.
- The preprocessor is the cross-GCC driver, not qcc: for qcc, -P means "preprocess to a file" (try.i), not "no line markers". perl-cross's probes run "$cpp -P" and read stdout, and Errno_pm.PL adds -P for GCC >= 5, so with qcc every signal-name probe and every errno value came back empty: perl had no %SIG names ("No such signal: SIGHUP") and an empty %! (hazard 131). Errno_pm.PL's first pass needs line markers, hence cpprun/cppstdin without -P. gccversion: perl-cross asks qcc and gets "0.0", so Errno_pm.PL did not add -P to its second pass either, and GCC split each '"EEXIST" [[17]]' line with line markers -- an empty %! again.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH perl`.

### 5. What it installs

2762 files under `/usr/local`, 30 programs.

```
/usr/local/bin/corelist
/usr/local/bin/cpan
/usr/local/bin/enc2xs
/usr/local/bin/encguess
/usr/local/bin/h2ph
/usr/local/bin/h2xs
/usr/local/bin/instmodsh
/usr/local/bin/json_pp
/usr/local/bin/libnetcfg
/usr/local/bin/perl
/usr/local/bin/perlbug
/usr/local/bin/perldoc
/usr/local/bin/perlivp
/usr/local/bin/perlthanks
/usr/local/bin/piconv
/usr/local/bin/pl2pm
...
```

The complete list is `stage/<arch>/.manifest/perl` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa perl
ls dists/8.0/*/ | grep '^perl-'
```

This writes `perl-<ver>-r0.tar.gz` (run time) and `perl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add perl
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/perl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/perl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl 5.40.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/perl-5.40.2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | perl 5.40.2-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/perl-5.40.2-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl 5.40.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/perl-5.40.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | perl 5.40.2-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/perl-5.40.2-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **5.40.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add perl=<version>`), sources in `ported/perl/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates perl`; port one with `scripts/port-update perl`.

<details><summary>recipe.sh</summary>

```bash
VERSION=5.40.2
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt
DEPS="libxcrypt"
TARBALL=https://www.cpan.org/src/5.0/perl-5.40.2.tar.gz
SHA256=10d4647cfbb543a7f9ae3e5f6851ec49305232ea7621aed24c7cfbb0bef4b70d
BUILDSYS=custom
# Perl cannot cross compile with its own Configure (it runs probes on the
# target). perl-cross replaces Configure with a cross-capable one: its tree is
# unpacked over the perl source (do_fetch). perl-cross knows QNX (osname
# "nto", cnf/hints/nto) since 1.6.2 -- the change qnx-ports carried for 1.6.1
# went upstream -- so only the perl patch in patches/ remains. This is the perl
# version the QNX Everywhere 8.0.4r1 image ships; it also covers qnx-ports'
# perl5 port.
PERLCROSS_URL=https://github.com/arsv/perl-cross/releases/download/1.6.2/perl-cross-1.6.2.tar.gz
PERLCROSS_SHA256=131f7496152ee32067dbac2bc9b44b2f582fc778140e545701b3b2faee782f1d
do_fetch() {
  local tb="$WS/dist/perl-5.40.2.tar.gz" pc="$WS/dist/perl-cross-1.6.2.tar.gz"
  mkdir -p "$WS/dist" "$SRCROOT"
  [ -f "$tb" ] || curl -fsSL -o "$tb" "$TARBALL" || return 1
  [ -f "$pc" ] || curl -fsSL -o "$pc" "$PERLCROSS_URL" || return 1
  echo "$SHA256  $tb" | sha256sum -c --status || { echo "perl checksum mismatch"; return 1; }
  echo "$PERLCROSS_SHA256  $pc" | sha256sum -c --status || { echo "perl-cross checksum mismatch"; return 1; }
  # perl first, perl-cross over it; patches/ then apply to the combined tree.
  tar xf "$tb" -C "$SRCROOT" --strip-components=1 && tar xf "$pc" -C "$SRCROOT" --strip-components=1
}
# The preprocessor is the cross-GCC driver, not qcc: for qcc, -P means
# "preprocess to a file" (try.i), not "no line markers". perl-cross's probes
# run "$cpp -P" and read stdout, and Errno_pm.PL adds -P for GCC >= 5, so with
# qcc every signal-name probe and every errno value came back empty: perl had
# no %SIG names ("No such signal: SIGHUP") and an empty %! (hazard 131).
# Errno_pm.PL's first pass needs line markers, hence cpprun/cppstdin
# without -P. gccversion: perl-cross asks qcc and gets "0.0", so Errno_pm.PL
# did not add -P to its second pass either, and GCC split each
# '"EEXIST" [[17]]' line with line markers -- an empty %! again.
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" -Dcc="qcc -V$QCC_TARGET" -Dld="qcc -V$QCC_TARGET" \
    -Dar="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" -Dccflags="-D_QNX_SOURCE" -Duseshrplib -Dusethreads \
    -Dusrinc="$QNX_TARGET_SDP/usr/include" -Dincpth="$QNX_TARGET_SDP/usr/include" \
    -Dcpp="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E -P" -Dgccversion="$("$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc" -dumpversion)" \
    -Dcpprun="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" -Dcppstdin="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
    -Dlibpth="$QNX_TARGET/$QNX_ARCH/lib $QNX_TARGET/$QNX_ARCH/usr/lib"
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
