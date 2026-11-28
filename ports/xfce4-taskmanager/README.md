# xfce4-taskmanager 1.6.0

> Xfce task manager: processes with CPU and memory use, end/kill/renice, CPU and memory graphs

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/apps/xfce4-taskmanager/1.6/xfce4-taskmanager-1.6.0.tar.xz |
| SHA-256 | `29bdc7840ab8b9025f6c0e456a83a31090d1c9fd9e26b359baa4a4010cfb0b90` |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`gtk3`](../gtk3/) [`cairo`](../cairo/) [`libxmu`](../libxmu/) [`libxfce4util`](../libxfce4util/) [`libxfce4ui`](../libxfce4ui/) [`xfconf`](../xfconf/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gettext`](../gettext/) [`glib`](../glib/) [`gtk3`](../gtk3/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`xfconf`](../xfconf/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfce4_taskmanager`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `xfce4-taskmanager` and `xfce4-taskmanager-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfce4-taskmanager` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (65 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib gtk3 cairo libxmu libxfce4util libxfce4ui xfconf
```

<details><summary>Full build chain (65)</summary>

[`at-spi2-core`](../at-spi2-core/) [`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`dbus`](../dbus/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`fribidi`](../fribidi/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`graphite2`](../graphite2/) [`gtk3`](../gtk3/) [`harfbuzz`](../harfbuzz/) [`icu`](../icu/) [`libepoxy`](../libepoxy/) [`libffi`](../libffi/) [`libice`](../libice/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libsm`](../libsm/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxcomposite`](../libxcomposite/) [`libxcursor`](../libxcursor/) [`libxdamage`](../libxdamage/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxfce4ui`](../libxfce4ui/) [`libxfce4util`](../libxfce4util/) [`libxfixes`](../libxfixes/) [`libxft`](../libxft/) [`libxi`](../libxi/) [`libxinerama`](../libxinerama/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxmu`](../libxmu/) [`libxrandr`](../libxrandr/) [`libxrender`](../libxrender/) [`libxt`](../libxt/) [`libxtst`](../libxtst/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`pango`](../pango/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xfconf`](../xfconf/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/apps/xfce4-taskmanager/1.6/xfce4-taskmanager-1.6.0.tar.xz
mkdir -p dist src/xfce4-taskmanager
curl -fL -o dist/$(basename $URL) $URL
echo "29bdc7840ab8b9025f6c0e456a83a31090d1c9fd9e26b359baa4a4010cfb0b90  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfce4-taskmanager --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfce4-taskmanager < ports/xfce4-taskmanager/patches/0001-qnx-backend.patch
```

**[`0001-qnx-backend.patch`](patches/0001-qnx-backend.patch)** — xfce4-taskmanager: a QNX backend (src/task-manager-qnx.c).

> The task manager reads processes, CPU and memory through one source file per
> operating system (Linux /proc, the BSDs' sysctl, Solaris kstat, Darwin);
> meson.build picks it by host_machine.system() and has none for QNX, so the
> build stopped there. task-manager-qnx.c implements the five functions of
> task-manager.h with what QNX Neutrino 8 provides:
>
> - processes: the numeric entries of /proc; devctl(DCMD_PROC_INFO) on
>   /proc/<pid>/ctl gives the parent, uid, base priority and the user and
>   system times in nanoseconds; /proc/<pid>/exefile the name, cmdline the
>   arguments (NUL-separated), vmstat the resident and mapped sizes (pages);
>   the state is "R" when a thread is RUNNING or READY (DCMD_PROC_TIDSTATUS),
>   else "S";
> - CPU: the sum of all processes' times between two samples, against the
>   elapsed time times the number of CPUs (the system page) -- the same scale
>   as the Linux backend's jiffies; the time of procnto's idle threads
>   (process 1, priority 0) is taken off (it is accounted as procnto's user
>   time, so procnto showed 99 % and the system 100 % busy);
> - procnto (process 1) is reported as its own parent; it is given parent 0,
>   as on Linux: the process tree model otherwise looped for ever on the cycle
>   (the window never appeared, the main thread at 100 %);
> - memory: RAM from the system page's "ram" entries, free memory from the
>   size of /proc; no swap.
>
> Sending signals and changing priorities stay in task-manager.c (kill(),
> setpriority()), which QNX has.

Touches: `meson.build`, `src/task-manager-qnx.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/xfce4-taskmanager src/xfce4-taskmanager \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dx11=disabled \
      -Dwnck=disabled
ninja -C build/$QNX_ARCH/xfce4-taskmanager
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/xfce4-taskmanager install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- xfce4-taskmanager: the Xfce task manager -- the process list (tree, CPU, memory, priority; end, kill, stop, continue and renice from the menu) with CPU and memory graphs. Raspberry Pi OS has lxtask in this place.
- libxmu: meson.build requires it, the program does not link it (X11 code is off).
- patches/0001: the QNX backend (processes from /proc and devctl()). Wayland only, as the rest of this Xfce: no X11, no libwnck (window icons and names of the processes' windows, X11 only).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfce4-taskmanager`.

### 5. What it installs

71 files under `/usr/local`, 1 programs.

```
/usr/local/bin/xfce4-taskmanager
```

The complete list is `stage/<arch>/.manifest/xfce4-taskmanager` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfce4-taskmanager
ls dists/8.0/*/ | grep '^xfce4-taskmanager-'
```

This writes `xfce4-taskmanager-<ver>-r0.tar.gz` (run time) and `xfce4-taskmanager-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfce4-taskmanager
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfce4-taskmanager-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfce4-taskmanager-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfce4_taskmanager`](../../tests/src/) links xfce4-taskmanager the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfce4-taskmanager`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfce4-taskmanager
/usr/local/bin/qnx-port-tests/test_xfce4_taskmanager
```

```
== xfce4-taskmanager ==
  ok   ...
PASS xfce4-taskmanager
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfce4_taskmanager` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfce4_taskmanager`](../../tests/src/) -- a program linked against xfce4-taskmanager the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> xfce4-taskmanager`.

## Versions

This recipe builds **1.6.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfce4-taskmanager=<version>`), sources in `ported/xfce4-taskmanager/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfce4-taskmanager`; port one with `scripts/port-update xfce4-taskmanager`.

<details><summary>recipe.sh</summary>

```bash
# xfce4-taskmanager: the Xfce task manager -- the process list (tree, CPU,
# memory, priority; end, kill, stop, continue and renice from the menu) with
# CPU and memory graphs. Raspberry Pi OS has lxtask in this place.
VERSION=1.6.0
TARBALL=https://archive.xfce.org/src/apps/xfce4-taskmanager/1.6/xfce4-taskmanager-1.6.0.tar.xz
SHA256=29bdc7840ab8b9025f6c0e456a83a31090d1c9fd9e26b359baa4a4010cfb0b90
# libxmu: meson.build requires it, the program does not link it (X11 code is off).
DEPS="glib gtk3 cairo libxmu libxfce4util libxfce4ui xfconf"
RUNTIME_DEPS="libxfce4ui xfconf"
BUILDSYS=meson
# patches/0001: the QNX backend (processes from /proc and devctl()).
# Wayland only, as the rest of this Xfce: no X11, no libwnck (window icons and
# names of the processes' windows, X11 only).
MESON_ARGS_EXTRA=( -Dx11=disabled -Dwnck=disabled )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
