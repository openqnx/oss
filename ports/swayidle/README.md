# swayidle 1.9.1

> Idle management daemon for wlroots Wayland compositors (runs commands on idle and resume)

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/swaywm/swayidle.git (tag `v1.9.1`) |
| Build system | Meson |
| QNX patches | 1 |
| Build dependencies | [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) |
| Run-time dependencies | [`wayland`](../wayland/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_swayidle`](../../tests/src/) |

## Install and remove on a target

Not published as a package yet: build it as below, then `scripts/qnxpkg` makes `swayidle` and `swayidle-dev`, which install and remove like every other package ([INSTALL.md](../../INSTALL.md), [docs/packages.md](../../docs/packages.md)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild swayidle` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (18 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild wayland wayland-protocols
```

<details><summary>Full build chain (18)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxml2`](../libxml2/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`wayland`](../wayland/) [`wayland-protocols`](../wayland-protocols/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b v1.9.1 --recurse-submodules --shallow-submodules \
    https://github.com/swaywm/swayidle.git src/swayidle
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/swayidle < ports/swayidle/patches/0001-qnx-wordexp-replacement.patch
```

**[`0001-qnx-wordexp-replacement.patch`](patches/0001-qnx-wordexp-replacement.patch)** — swayidle: parse the configuration without wordexp() on QNX.

> QNX libc declares wordexp() but it is a stub: it returns -1 with ENOSYS
> for every string. swayidle splits each configuration line with it (and
> expands the configuration path), so with a configuration file it logged
> "Shell expansion error ... unknown error (-1)" and exited. Use a small
> replacement on QNX: words split on blanks, single and double quotes and
> backslashes honoured, a leading ~ and $VAR / ${VAR} expanded. The
> commands themselves are still run by sh -c.

Touches: `main.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
meson setup build/$QNX_ARCH/swayidle src/swayidle \
      --cross-file $QNX_MESON_DIR/meson-$QNX_ARCH.ini \
      --native-file $QNX_MESON_DIR/meson-native.ini \
      --prefix=/usr/local --buildtype=release --default-library=both \
      -Dlogind=disabled \
      -Dman-pages=disabled \
      -Dwerror=false \
      -Dzsh-completions=true \
      -Dbash-completions=true \
      -Dfish-completions=true
ninja -C build/$QNX_ARCH/swayidle
DESTDIR=$STAGE ninja -C build/$QNX_ARCH/swayidle install
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- swayidle: the idle management daemon of wlroots compositors such as labwc. It asks the compositor (ext-idle-notify-v1) to tell it when there has been no input for a given time and runs commands then and when activity resumes: swayidle -w timeout 300 'swaylock -f' timeout 600 'wlopm --off "*"' resume 'wlopm --on "*"' SIGUSR1 runs the idle commands at once.
- wayland: libwayland-client, and libwayland-server for its event loop.
- - logind (systemd/elogind): the before-sleep, after-resume, lock and unlock events come from logind over sd-bus; QNX has neither, so only the timeout and resume commands are built; - no man page (scdoc is not a build tool here); the shell completions are data; - werror=false: upstream builds with -Werror, and the QNX headers under -D_POSIX_C_SOURCE with GCC 12 are not warning-free on both architectures.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH swayidle`.

### 5. What it installs

4 files under `/usr/local`, 1 programs.

```
/usr/local/bin/swayidle
```

The complete list is `stage/<arch>/.manifest/swayidle` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa swayidle
ls dists/8.0/*/ | grep '^swayidle-'
```

This writes `swayidle-<ver>-r0.tar.gz` (run time) and `swayidle-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add swayidle
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/swayidle-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/swayidle-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_swayidle`](../../tests/src/) links swayidle the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-swayidle`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-swayidle
/usr/local/bin/qnx-port-tests/test_swayidle
```

```
== swayidle ==
  ok   ...
PASS swayidle
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_swayidle` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_swayidle`](../../tests/src/) -- a program linked against swayidle the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Not run on a target yet: `scripts/verify-on-target -t "<target>" <target> swayidle`.

## Versions

This recipe builds **1.9.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add swayidle=<version>`), sources in `ported/swayidle/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates swayidle`; port one with `scripts/port-update swayidle`.

<details><summary>recipe.sh</summary>

```bash
# swayidle: the idle management daemon of wlroots compositors such as labwc.
# It asks the compositor (ext-idle-notify-v1) to tell it when there has been no
# input for a given time and runs commands then and when activity resumes:
#   swayidle -w timeout 300 'swaylock -f' timeout 600 'wlopm --off "*"' resume 'wlopm --on "*"'
# SIGUSR1 runs the idle commands at once.
VERSION=1.9.1; GIT=https://github.com/swaywm/swayidle.git; REF=v1.9.1
# wayland: libwayland-client, and libwayland-server for its event loop.
DEPS="wayland wayland-protocols"
BUILDSYS=meson
# - logind (systemd/elogind): the before-sleep, after-resume, lock and unlock
#   events come from logind over sd-bus; QNX has neither, so only the timeout
#   and resume commands are built;
# - no man page (scdoc is not a build tool here); the shell completions are data;
# - werror=false: upstream builds with -Werror, and the QNX headers under
#   -D_POSIX_C_SOURCE with GCC 12 are not warning-free on both architectures.
MESON_ARGS_EXTRA=( -Dlogind=disabled -Dman-pages=disabled -Dwerror=false
                   -Dzsh-completions=true -Dbash-completions=true -Dfish-completions=true )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
