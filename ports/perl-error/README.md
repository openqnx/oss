# perl-error 0.17030

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
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install perl-error
oss-get install perl-error-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

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
./scripts/qnxpkg -a $QNX_ARCH perl-error
ls repo/8.0/*/ | grep '^perl-error-'
```

This writes `perl-error-<ver>-r0.tar.gz` (run time) and `perl-error-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install perl-error
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/perl-error-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/perl-error-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

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
