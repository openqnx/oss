# perl 5.40.2

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.cpan.org/src/5.0/perl-5.40.2.tar.gz |
| SHA-256 | `10d4647cfbb543a7f9ae3e5f6851ec49305232ea7621aed24c7cfbb0bef4b70d` |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`libxcrypt`](../libxcrypt/) |
| Run-time dependencies | [`libxcrypt`](../libxcrypt/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install perl
oss-get install perl-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/perl < ports/perl/patches/0001-errno-target-osname.patch
```

**[`0001-errno-target-osname.patch`](patches/0001-errno-target-osname.patch)** — Errno_pm.PL: decide by the target OS, not the build host's.

> When cross compiling, Errno_pm.PL runs under the build host's miniperl,
> where $^O is "linux". It then takes its Linux path, preprocesses the host's
> glibc <errno.h> with the QNX compiler ("features.h: No such file") and finds
> no error definitions. $Config{osname} is the target's ("nto").

Touches: `ext/Errno/Errno_pm.PL` · Upstream status: QNX-specific

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
    -Dcpprun="qcc -V$QCC_TARGET -E" -Dcppstdin="qcc -V$QCC_TARGET -E" \
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
- The preprocessor is given without perl-cross's default -P: Errno_pm.PL finds <errno.h> by the line markers in its output ("No error definitions found" without them).

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
./scripts/qnxpkg -a $QNX_ARCH perl
ls repo/8.0/*/ | grep '^perl-'
```

This writes `perl-<ver>-r0.tar.gz` (run time) and `perl-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install perl
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/perl-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/perl-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
# The preprocessor is given without perl-cross's default -P: Errno_pm.PL
# finds <errno.h> by the line markers in its output ("No error definitions
# found" without them).
do_configure() {
  cp -a "$S/." "$B/" && cd "$B" && \
  ./configure --target="$QNX_TRIPLE" --prefix="$PREFIX" -Dcc="qcc -V$QCC_TARGET" -Dld="qcc -V$QCC_TARGET" \
    -Dar="$QNX_HOST/usr/bin/nto${QNX_CPU}-ar" -Dccflags="-D_QNX_SOURCE" -Duseshrplib -Dusethreads \
    -Dusrinc="$QNX_TARGET_SDP/usr/include" -Dincpth="$QNX_TARGET_SDP/usr/include" \
    -Dcpprun="qcc -V$QCC_TARGET -E" -Dcppstdin="qcc -V$QCC_TARGET -E" \
    -Dlibpth="$QNX_TARGET/$QNX_ARCH/lib $QNX_TARGET/$QNX_ARCH/usr/lib"
}
do_build() { make -C "$B" -j"$(nproc)"; }
do_install() { make -C "$B" DESTDIR="$STAGE" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
