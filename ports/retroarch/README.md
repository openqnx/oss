# retroarch 1.15-git-20230408

> RetroArch, the libretro frontend for emulators and games

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/RetroPie/RetroArch.git |
| Build system | custom (see recipe) |
| QNX patches | 4 |
| Build dependencies | [`sdl2`](../sdl2/) [`freetype`](../freetype/) [`zlib`](../zlib/) [`fontconfig`](../fontconfig/) [`fribidi`](../fribidi/) [`libass`](../libass/) [`openssl`](../openssl/) |
| Run-time dependencies | [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`libass`](../libass/) [`libxkbcommon`](../libxkbcommon/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_retroarch`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `retroarch` (programs, shared libraries, run-time data) and `retroarch-dev` (headers, pkg-config/CMake files, static libraries; needs `retroarch`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`retroarch`](https://openqnx.github.io/oss/package/x86_64/retroarch.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/retroarch.html)) · [`retroarch-dev`](https://openqnx.github.io/oss/package/x86_64/retroarch-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/retroarch-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add retroarch` (dependencies come along); one version: `apk add retroarch=<version>` | `apk del retroarch` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install retroarch` (dependencies first) | `oss-get remove retroarch` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`retroarch-dev` the same way, by its own name (`apk add retroarch-dev`, `oss-get install retroarch-dev`, by hand with its own file and list `retroarch-dev.list`); remove it before `retroarch`, which it needs.

```sh
# on the QNX target, as root
apk add retroarch retroarch-dev
apk add qnx-test-retroarch && /usr/local/bin/qnx-port-tests/test_retroarch    # its on-target test, from tests-8.0-<arch>-m-z
apk del retroarch retroarch-dev qnx-test-retroarch
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/retroarch.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/retroarch.list
rm var/lib/manual/retroarch.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild retroarch` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (30 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild sdl2 freetype zlib fontconfig fribidi libass openssl
```

<details><summary>Full build chain (30)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libass`](../libass/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`python3`](../python3/) [`readline`](../readline/) [`sdl2`](../sdl2/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 --recurse-submodules --shallow-submodules \
    https://github.com/RetroPie/RetroArch.git src/retroarch
```

### 3. Apply the QNX patches

4 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/retroarch < ports/retroarch/patches/0001-qnx-from-qnx-ports.patch
patch -p1 -d src/retroarch < ports/retroarch/patches/0002-qnx-no-libstdcxx.patch
patch -p1 -d src/retroarch < ports/retroarch/patches/0003-qnx-getopt-restart.patch
patch -p1 -d src/retroarch < ports/retroarch/patches/0004-qnx-keep-config-option.patch
```

**[`0001-qnx-from-qnx-ports.patch`](patches/0001-qnx-from-qnx-ports.patch)** — updated drivers

> Imported from qnx-ports/RetroArch (d625ca376267...qnx_master); original authorship kept.
>
>
> ---
>  Makefile.common                   |    9 +
>  frontend/drivers/platform_qnx.c   |   57 +-
>  frontend/drivers/platform_unix.c  |    4 +-
>  gfx/drivers_context/qnx_ctx.c     |  111 ++-
>  input/drivers/qnx_input.c         | 1176 ++++++++++++-----------------
>  input/drivers/qnx_input.h         |   95 +++
>  input/drivers/qnx_input_old.c     |  857 +++++++++++++++++++++
>  input/drivers_joypad/qnx_joypad.c |   10 +-
>  qb/config.libs.sh                 |    4 +
>  qb/qb.system.sh                   |    1 +
>  10 files changed, 1535 insertions(+), 789 deletions(-)
>  create mode 100644 input/drivers/qnx_input.h
>  create mode 100644 input/drivers/qnx_input_old.c

Touches: `Makefile.common`, `frontend/drivers/platform_qnx.c`, `frontend/drivers/platform_unix.c`, `gfx/drivers_context/qnx_ctx.c`, `input/drivers/qnx_input.c`, `input/drivers/qnx_input.h` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-no-libstdcxx.patch`](patches/0002-qnx-no-libstdcxx.patch)** — Do not link GNU libstdc++ on QNX.

> Makefile.common adds -lstdc++ for the switchres code unconditionally, which
> is GNU's C++ runtime. QNX's q++ links its own, libc++, and the objects are
> compiled against it: retroarch ended up loading both libc++.so.2 and the
> SDP's libstdc++.so.6 (scripts/check-sdp-leak), two C++ runtimes with their
> own exception handling and std:: types in one process. Leave it out on QNX.

Touches: `Makefile.common` · Upstream status: QNX-specific

**[`0003-qnx-getopt-restart.patch`](patches/0003-qnx-getopt-restart.patch)** — QNX getopt_long has no GNU "optind = 0" restart: --config was ignored

> retroarch_parse_input_and_config() starts its first pass with optind = 0,
> which glibc takes as "re-initialise". QNX's BSD-derived getopt_long does
> not, so the first pass parsed no option at all, and the ones only it
> handles (--config, --appendconfig) were silently ignored: RetroArch always
> read <cwd>/rarch-shared/retroarch.cfg. Restart with optind = 1 and
> optreset = 1 on QNX (both passes).

Touches: `retroarch.c` · Upstream status: QNX-specific

**[`0004-qnx-keep-config-option.patch`](patches/0004-qnx-keep-config-option.patch)** — Keep a --config path over the frontend default config path

> config_set_defaults() runs after the first argument pass and sets
> RARCH_PATH_CONFIG from g_defaults.path_config whenever the frontend filled
> that in. The QNX frontend does (<cwd>/rarch-shared/retroarch.cfg), so
> --config was always replaced and RetroArch said "Config not found" for its
> own default. Only use the default when no path was given.

Touches: `configuration.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/retroarch; B=build/$QNX_ARCH/retroarch; mkdir -p $B
```

```bash
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc" CXX="q++" CFLAGS="-D_QNX_SOURCE -std=c17 -V$QCC_TARGET -I$STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -std=c++17 -V${QCC_TARGET}_cxx -Wno-deprecated-definitions -I$STAGE$PREFIX/include" \
  CPPFLAGS="-D_QNX_SOURCE" LDFLAGS="-V${QCC_TARGET}_cxx -L$STAGE$PREFIX/lib" \
  PKG_CONF_PATH=pkg-config \
  ./configure --host="$QNX_TRIPLE-" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --disable-x11 --disable-wayland --disable-builtinzlib --disable-ffmpeg; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)" EGL_LIBS=-lEGL; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" EGL_LIBS=-lEGL install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- RetroArch, the libretro frontend, as packaged by RetroPie, at the upstream commit qnx-ports' QNX port is based on (patches/: QNX platform support -- Screen/EGL video, input, QNX build settings). Part of the RetroPie stack.
- RetroArch's configure is its own (not autoconf) and builds in the source tree.
- LDFLAGS -V..._cxx: the program is linked by q++, and the plain -V variant is GNU libstdc++'s -- next to the libc++ every C++ object here is compiled against, so the binary loaded both C++ runtimes (scripts/check-sdp-leak). PKG_CONF_PATH: configure looks for <triple>-pkg-config; the tree's pkg-config (PKG_CONFIG_LIBDIR/SYSROOT from env.sh) is the one to use. --disable-ffmpeg: this RetroArch (2023, the RetroPie revision qnx-ports ports) uses the FFmpeg API before 7.0 (AVCodecContext::channel_layout); the tree's FFmpeg is 7. It only provides the ffmpeg core and recording. --disable-builtinzlib: the zlib component (a DEPS entry), not the copy in deps/libz, whose prototypes clash with the staged zlib.h. --disable-x11 --disable-wayland: on QNX RetroArch draws through Screen/SDL2. Its Wayland input maps Linux evdev key codes (<linux/input.h>); its X11 drivers include the XShm one, which needs SysV shared memory (<sys/shm.h>) that QNX does not have. EGL_LIBS=-lEGL: configure finds QNX's EGL (HAVE_EGL) through its headers but has no pkg-config file to take the library from, so the link missed it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH retroarch`.

### 5. What it installs

10 files under `/usr/local`, 2 programs.

```
/usr/local/bin/retroarch
/usr/local/bin/retroarch-cg2glsl
```

The complete list is `stage/<arch>/.manifest/retroarch` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa retroarch
ls dists/8.0/*/ | grep '^retroarch-'
```

This writes `retroarch-<ver>-r0.tar.gz` (run time) and `retroarch-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add retroarch
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/retroarch-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/retroarch-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_retroarch`](../../tests/src/) links retroarch the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-retroarch`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-retroarch
/usr/local/bin/qnx-port-tests/test_retroarch
```

```
== retroarch ==
  ok   ...
PASS retroarch
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_retroarch` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_retroarch`](../../tests/src/) -- a program linked against retroarch the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | retroarch 1.15_git20230408-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/retroarch-1.15_git20230408-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | retroarch 1.15_git20230408-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/retroarch-1.15_git20230408-r2.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | retroarch 1.15_git20230408-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/retroarch-1.15_git20230408-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | retroarch 1.15_git20230408-r2 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/retroarch-1.15_git20230408-r2.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.15-git-20230408**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add retroarch=<version>`), sources in `ported/retroarch/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates retroarch`; port one with `scripts/port-update retroarch`.

<details><summary>recipe.sh</summary>

```bash
# RetroArch, the libretro frontend, as packaged by RetroPie, at the upstream
# commit qnx-ports' QNX port is based on (patches/: QNX platform support --
# Screen/EGL video, input, QNX build settings). Part of the RetroPie stack.
VERSION=1.15-git-20230408; GIT=https://github.com/RetroPie/RetroArch.git; COMMIT=d625ca376267cef2c304a8a8f033f21003c8df02
DEPS="sdl2 freetype zlib fontconfig fribidi libass openssl"
BUILDSYS=custom
# RetroArch's configure is its own (not autoconf) and builds in the source tree.
do_configure() { cp -a "$S/." "$B/" && cd "$B" && \
  CC="qcc" CXX="q++" CFLAGS="-D_QNX_SOURCE -std=c17 -V$QCC_TARGET -I$STAGE$PREFIX/include" \
  CXXFLAGS="-D_QNX_SOURCE -std=c++17 -V${QCC_TARGET}_cxx -Wno-deprecated-definitions -I$STAGE$PREFIX/include" \
  CPPFLAGS="-D_QNX_SOURCE" LDFLAGS="-V${QCC_TARGET}_cxx -L$STAGE$PREFIX/lib" \
  PKG_CONF_PATH=pkg-config \
  ./configure --host="$QNX_TRIPLE-" --prefix="$PREFIX" --sysconfdir="$PREFIX/etc" --disable-x11 --disable-wayland --disable-builtinzlib --disable-ffmpeg; }
# LDFLAGS -V..._cxx: the program is linked by q++, and the plain -V variant is
# GNU libstdc++'s -- next to the libc++ every C++ object here is compiled
# against, so the binary loaded both C++ runtimes (scripts/check-sdp-leak).
# PKG_CONF_PATH: configure looks for <triple>-pkg-config; the tree's pkg-config
# (PKG_CONFIG_LIBDIR/SYSROOT from env.sh) is the one to use.
# --disable-ffmpeg: this RetroArch (2023, the RetroPie revision qnx-ports
# ports) uses the FFmpeg API before 7.0 (AVCodecContext::channel_layout); the
# tree's FFmpeg is 7. It only provides the ffmpeg core and recording.
# --disable-builtinzlib: the zlib component (a DEPS entry), not the copy in
# deps/libz, whose prototypes clash with the staged zlib.h.
# --disable-x11 --disable-wayland: on QNX RetroArch draws through Screen/SDL2.
# Its Wayland input maps Linux evdev key codes (<linux/input.h>); its X11 drivers
# include the XShm one, which needs SysV shared memory (<sys/shm.h>) that QNX
# does not have.
# EGL_LIBS=-lEGL: configure finds QNX's EGL (HAVE_EGL) through its headers but
# has no pkg-config file to take the library from, so the link missed it.
do_build() { make -C "$B" -j"$(nproc)" EGL_LIBS=-lEGL; }
do_install() { make -C "$B" DESTDIR="$STAGE" EGL_LIBS=-lEGL install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
