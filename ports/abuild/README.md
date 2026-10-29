# abuild 3.16.0

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/alpinelinux/abuild.git (tag `3.16.0`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) [`apk-tools`](../apk-tools/) |
| Run-time dependencies | [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install abuild
oss-get install abuild-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild abuild` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (4 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild openssl zlib apk-tools
```

<details><summary>Full build chain (4)</summary>

[`apk-tools`](../apk-tools/) [`openssl`](../openssl/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.16.0 --recurse-submodules --shallow-submodules \
    https://github.com/alpinelinux/abuild.git src/abuild
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/abuild < ports/abuild/patches/0001-qnx-from-qnx-ports.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — qnx: remove features not used by QNX

> - /usr merge, no longer using /bin, /sbin, /lib, /usr/sbin and /usr/libexec
>   - also applies to openrc
> - add qnx target triplets
> - remove -pyc auto splitting
> - remove auto so dependency, only prompt a warning
> - remove capability detections
> - remove nftables auto splitting
> - remove multi-libc support
>
> ---
>  abuild-sudo.c   |  10 +--
>  abuild.in       | 193 +++++++++++++++++++++++++-----------------------
>  functions.sh.in |  62 +---------------
>  3 files changed, 107 insertions(+), 158 deletions(-)

Touches: `abuild-sudo.c`, `abuild.in`, `functions.sh.in`, `abuild.in`, `abuild.in`, `default.conf` … · Upstream status: QNX-specific (from qnx-ports)

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/abuild; B=build/$QNX_ARCH/abuild; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/"; }
```

```bash
do_build() { _mk abuild-tar abuild-gzsplit abuild-sudo abuild abuild-keygen abuild-sign newapkbuild abump apkgrel buildlab checkapk functions.sh; }
```

```bash
do_install() { _mk DESTDIR="$STAGE" install && rm -rf "$STAGE$PREFIX/share/man"; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- abuild: Alpine's APK package build tool (shell scripts plus small C helpers) "adapted for QNX": upstream 3.16.0 (GitHub mirror of gitlab.alpinelinux.org) with qnx-ports' QNX commits (patches/, the release QNX's aports builds, 3.16.0_p5; the image ships 3.15.0); the scripts expect a POSIX shell, apk-tools and the usual build utilities on the target.
- The man pages are made with scdoc, which the build host does not have; SCDOC=true leaves them empty, and they are not installed.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH abuild`.

### 5. What it installs

30 files under `/usr/local`, 19 programs.

```
/usr/local/bin/abuild
/usr/local/bin/abuild-addgroup
/usr/local/bin/abuild-adduser
/usr/local/bin/abuild-apk
/usr/local/bin/abuild-fetch
/usr/local/bin/abuild-gzsplit
/usr/local/bin/abuild-keygen
/usr/local/bin/abuild-rmtemp
/usr/local/bin/abuild-sign
/usr/local/bin/abuild-sudo
/usr/local/bin/abuild-tar
/usr/local/bin/abump
/usr/local/bin/apkbuild-cpan
/usr/local/bin/apkbuild-gem-resolver
/usr/local/bin/apkbuild-pypi
/usr/local/bin/apkgrel
...
```

The complete list is `stage/<arch>/.manifest/abuild` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH abuild
ls repo/8.0/*/ | grep '^abuild-'
```

This writes `abuild-<ver>-r0.tar.gz` (run time) and `abuild-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install abuild
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/abuild-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/abuild-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
# abuild: Alpine's APK package build tool (shell scripts plus small C
# helpers) "adapted for QNX": upstream 3.16.0 (GitHub mirror of
# gitlab.alpinelinux.org) with qnx-ports' QNX commits (patches/, the release
# QNX's aports builds, 3.16.0_p5; the image ships 3.15.0); the scripts expect a POSIX shell, apk-tools and the
# usual build utilities on the target.
VERSION=3.16.0; GIT=https://github.com/alpinelinux/abuild.git; REF=3.16.0
DEPS="openssl zlib apk-tools"
BUILDSYS=custom
# The man pages are made with scdoc, which the build host does not have;
# SCDOC=true leaves them empty, and they are not installed.
_mk() { make -C "$B" SCDOC=true CC="qcc -V$QCC_TARGET" CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include" \
  LDFLAGS="-L$STAGE$PREFIX/lib" prefix="$PREFIX" sysconfdir="$PREFIX/etc" "$@"; }
do_configure() { cp -a "$S/." "$B/"; }
do_build() { _mk abuild-tar abuild-gzsplit abuild-sudo abuild abuild-keygen abuild-sign newapkbuild abump apkgrel buildlab checkapk functions.sh; }
do_install() { _mk DESTDIR="$STAGE" install && rm -rf "$STAGE$PREFIX/share/man"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
