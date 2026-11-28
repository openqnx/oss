# swaylock 1.8.6

> Screen locker for wlroots Wayland compositors (ext-session-lock, PAM)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/swaywm/swaylock.git (tag `v1.8.6`) |
| Build system | Meson |
| QNX patches | 3 |
| Build dependencies | [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`libxkbcommon`](../libxkbcommon/) [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`openpam`](../openpam/) |
| Run-time dependencies | [`cairo`](../cairo/) [`gdk-pixbuf`](../gdk-pixbuf/) [`glib`](../glib/) [`libxkbcommon`](../libxkbcommon/) [`openpam`](../openpam/) [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_swaylock`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `swaylock` and `swaylock-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild swaylock` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (41 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild wayland wayland-protocols libxkbcommon cairo gdk-pixbuf openpam
```

<details><summary>Full build chain (41)</summary>

[`brotli`](../brotli/) [`bzip2`](../bzip2/) [`cairo`](../cairo/) [`expat`](../expat/) [`fontconfig`](../fontconfig/) [`freetype`](../freetype/) [`gdbm`](../gdbm/) [`gdk-pixbuf`](../gdk-pixbuf/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libjpeg-turbo`](../libjpeg-turbo/) [`libpng`](../libpng/) [`libunistring`](../libunistring/) [`libx11`](../libx11/) [`libxau`](../libxau/) [`libxcb`](../libxcb/) [`libxdmcp`](../libxdmcp/) [`libxext`](../libxext/) [`libxkbcommon`](../libxkbcommon/) [`libxml2`](../libxml2/) [`libxrender`](../libxrender/) [`lzo`](../lzo/) [`ncurses`](../ncurses/) [`openpam`](../openpam/) [`openssl`](../openssl/) [`pcre2`](../pcre2/) [`pixman`](../pixman/) [`pthread-stubs`](../pthread-stubs/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xcb-proto`](../xcb-proto/) [`xorgproto`](../xorgproto/) [`xtrans`](../xtrans/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.8.6 --recurse-submodules --shallow-submodules \
    https://github.com/swaywm/swaylock.git src/swaylock
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/swaylock < ports/swaylock/patches/0001-librt-is-optional.patch
patch -p1 -d src/swaylock < ports/swaylock/patches/0001-qnx-pam-policy-pam-qnx.patch
patch -p1 -d src/swaylock < ports/swaylock/patches/0002-qnx-no-sa-restart.patch
```

**[`0001-librt-is-optional.patch`](patches/0001-librt-is-optional.patch)** — build: librt is optional

> shm_open() and clock_gettime(), the reason librt is on the link line, are
> in libc on QNX (as on current glibc, musl and the BSDs), and QNX has no
> librt at all: the required find_library('rt') stopped the configuration.
> Link it where it exists.

Touches: `meson.build` · Upstream status: Pending

**[`0001-qnx-pam-policy-pam-qnx.patch`](patches/0001-qnx-pam-policy-pam-qnx.patch)** — swaylock: a PAM policy that works on a stock QNX image.

> The installed policy is "auth include login", but a QNX target built with
> mkqnximage has no /etc/pam.d at all (the SDP's etc/pam.d/login is not put
> into the image), so pam_start() failed and swaylock left as soon as it had
> locked: "pam_start failed", "Failed to read pw result". Name the module
> QNX's own login policy uses, pam_qnx.so, directly -- by its path in the
> image, /system/lib/dll/pam/pam_qnx.so (OpenPAM loads a module named with a
> "/" from that path, after checking it belongs to root and only root may
> write it; /system/lib/dll/pam is not on its module search path) -- and for
> all four facilities, as that policy does: with "auth" alone OpenPAM goes
> looking for the "other" policy for the rest, does not find one, and
> pam_start() fails (System error, ENOENT). pam_qnx.so links OpenPAM's
> libpam.so.2 from /usr/local/lib: swaylock needs the desktop session's
> LD_LIBRARY_PATH (oss-env.sh), which it runs with.

Touches: `pam/swaylock` · Upstream status: QNX-specific

**[`0002-qnx-no-sa-restart.patch`](patches/0002-qnx-no-sa-restart.patch)** — main: SA_RESTART does not exist on QNX

> The SIGUSR1 (unlock) handler is installed with sa_flags = SA_RESTART. QNX's
> <signal.h> has the flag commented out ("not supported yet"), so main.c did
> not compile; it is the only use in the source. Define it to 0 there. A
> system call interrupted by the signal then returns EINTR instead of being
> restarted: the event loop's poll() (loop.c) and the read/write of the
> password pipe (comm.c) already retry on EINTR, and the only signal handled
> this way is the one that ends the locker anyway.

Touches: `main.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/swaylock src/swaylock \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dpam=enabled \
      -Dgdk-pixbuf=enabled \
      -Dman-pages=disabled \
      -Dwerror=false \
      -Dzsh-completions=true \
      -Dbash-completions=true \
      -Dfish-completions=true
ninja -C build/$QNX_ARCH/swaylock
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/swaylock install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- swaylock: the screen locker of wlroots compositors such as labwc. It locks the session through ext-session-lock-v1 (the compositor shows nothing but the locker's surfaces and keeps the session locked even if the locker dies), draws a colour or an image with the typing indicator, and checks the password through PAM. Typically started by swayidle: swaylock -f -c 000000. SIGUSR1 unlocks.
- openpam: libpam.so.2 -- the same library and soname the SDP ships; the PAM modules that check the password are QNX's own (/usr/lib/pam_qnx.so). gdk-pixbuf: background images other than PNG.
- The keymap the compositor hands over is compiled by libxkbcommon (it needs the XKB data only for the compose table); a font for the indicator text.
- - pam=enabled: the policy is etc/pam.d/swaylock ("auth include login"), which OpenPAM finds in /usr/local/etc/pam.d after /etc/pam.d; "login" is QNX's own policy (pam_qnx.so). The alternative, the shadow backend, needs a setuid-root binary and <crypt.h>; - no man page (scdoc is not a build tool here); the shell completions are data; - werror=false: upstream builds with -Werror.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH swaylock`.

### 5. What it installs

5 files under `/usr/local`, 1 programs.

```
/usr/local/bin/swaylock
```

The complete list is `stage/<arch>/.manifest/swaylock` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa swaylock
ls dists/8.0/*/ | grep '^swaylock-'
```

This writes `swaylock-<ver>-r0.tar.gz` (run time) and `swaylock-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add swaylock
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/swaylock-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/swaylock-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_swaylock`](../../tests/src/) links swaylock the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-swaylock`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-swaylock
/usr/local/bin/qnx-port-tests/test_swaylock
```

```
== swaylock ==
  ok   ...
PASS swaylock
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_swaylock` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_swaylock`](../../tests/src/) -- a program linked against swaylock the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> swaylock`.

## Versions

This recipe builds **1.8.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add swaylock=<version>`), sources in `ported/swaylock/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates swaylock`; port one with `scripts/port-update swaylock`.

<details><summary>recipe.sh</summary>

```bash
# swaylock: the screen locker of wlroots compositors such as labwc. It locks
# the session through ext-session-lock-v1 (the compositor shows nothing but
# the locker's surfaces and keeps the session locked even if the locker dies),
# draws a colour or an image with the typing indicator, and checks the
# password through PAM. Typically started by swayidle: swaylock -f -c 000000.
# SIGUSR1 unlocks.
VERSION=1.8.6; GIT=https://github.com/swaywm/swaylock.git; REF=v1.8.6
# openpam: libpam.so.2 -- the same library and soname the SDP ships; the
# PAM modules that check the password are QNX's own (/usr/lib/pam_qnx.so).
# gdk-pixbuf: background images other than PNG.
DEPS="wayland wayland-protocols libxkbcommon cairo gdk-pixbuf openpam"
# The keymap the compositor hands over is compiled by libxkbcommon (it needs
# the XKB data only for the compose table); a font for the indicator text.
RUNTIME_DEPS="xkeyboard-config font-dejavu"
BUILDSYS=meson
# - pam=enabled: the policy is etc/pam.d/swaylock ("auth include login"),
#   which OpenPAM finds in /usr/local/etc/pam.d after /etc/pam.d; "login" is
#   QNX's own policy (pam_qnx.so). The alternative, the shadow backend, needs
#   a setuid-root binary and <crypt.h>;
# - no man page (scdoc is not a build tool here); the shell completions are data;
# - werror=false: upstream builds with -Werror.
MESON_ARGS_EXTRA=( -Dpam=enabled -Dgdk-pixbuf=enabled -Dman-pages=disabled -Dwerror=false
                   -Dzsh-completions=true -Dbash-completions=true -Dfish-completions=true )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
