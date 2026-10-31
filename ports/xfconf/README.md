# xfconf 4.20.0

> Hierarchical configuration storage system for Xfce

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/xfconf/4.20/xfconf-4.20.0.tar.bz2 |
| SHA-256 | `8bc43c60f1716b13cf35fc899e2a36ea9c6cdc3478a8f051220eef0f53567efd` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) [`libxfce4util`](../libxfce4util/) [`dbus`](../dbus/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) [`libxfce4util`](../libxfce4util/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_xfconf`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add xfconf
apk add xfconf-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-xfconf && /usr/local/bin/qnx-port-tests/test_xfconf    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install xfconf`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild xfconf` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (12 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib libxfce4util dbus
```

<details><summary>Full build chain (12)</summary>

[`bzip2`](../bzip2/) [`dbus`](../dbus/) [`expat`](../expat/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`libxfce4util`](../libxfce4util/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/xfconf/4.20/xfconf-4.20.0.tar.bz2
mkdir -p dist src/xfconf
curl -fL -o dist/$(basename $URL) $URL
echo "8bc43c60f1716b13cf35fc899e2a36ea9c6cdc3478a8f051220eef0f53567efd  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/xfconf --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/xfconf < ports/xfconf/patches/0001-qnx-sa-restart.patch
```

**[`0001-qnx-sa-restart.patch`](patches/0001-qnx-sa-restart.patch)** — Define SA_RESTART as 0 on QNX, where <signal.h> leaves it commented out.

> QNX 8.0's <signal.h> carries SA_RESTART commented out ("not supported yet"):
> an interrupted kernel call returns EINTR rather than restarting.  This code
> only passes the flag to sigaction(); nothing branches on it, and the callers
> already retry on EINTR, so 0 is a true no-op rather than a behaviour change.

Touches: `xfconfd/main.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/xfconf && cd build/$QNX_ARCH/xfconf
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/xfconf/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --disable-gtk-doc \
    --disable-introspection \
    --disable-linker-opts
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- The release tarball, not the git tag: Xfce keeps its generated sources (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
- GDBUS_CODEGEN is exported centrally by scripts/qnxbuild -- xfconf, xfce4-panel and tumbler all need it.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH xfconf`.

### 5. What it installs

75 files under `/usr/local`: 2 shared-library entries, 1 programs, 5 headers.

```
/usr/local/lib/libxfconf-0.so
/usr/local/lib/libxfconf-0.so.3
/usr/local/bin/xfconf-query
```

The complete list is `stage/<arch>/.manifest/xfconf` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa xfconf
ls dists/8.0/*/ | grep '^xfconf-'
```

This writes `xfconf-<ver>-r0.tar.gz` (run time) and `xfconf-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add xfconf
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/xfconf-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/xfconf-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_xfconf`](../../tests/src/) links xfconf the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-xfconf`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-xfconf
/usr/local/bin/qnx-port-tests/test_xfconf
```

```
== xfconf ==
  ok   ...
PASS xfconf
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_xfconf` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_xfconf`](../../tests/src/) -- a program linked against xfconf the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - scratch directory
   - xfconf-query --version: 4.20.0
   - xfconfd --version: 4.20.0
   - private dbus-daemon listening on a UNIX socket
   - xfconfd serves the seeded channel: /greeting
   - seeded int /answer = 42
   - seeded nested bool /nested/flag = true
   - xfconf-query -n creates /app/name in a new channel
   - string property created and read back
   - int property created, then changed
   - array property (2 strings)
   - -l lists the channel's properties
   - -l lists both channels (after the delayed save)
   - -r resets a property (now reported missing)
   - xfconf_init on the private bus
   - libxfconf reads what xfconf-query wrote
   - libxfconf get_int
   - libxfconf set double/bool/uint
   - xfconf-query reads the double back
   - ... the bool
   - ... the uint
   - string list round trip
   - has_property
   - xfconf_list_channels includes qnx-seed
   - property-changed signal for xfconf-query's change
   - ... and 3 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | xfconf 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/xfconf-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | xfconf 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/xfconf-4.20.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add xfconf=<version>`), sources in `ported/xfconf/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates xfconf`; port one with `scripts/port-update xfconf`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/xfconf/4.20/xfconf-4.20.0.tar.bz2
SHA256=8bc43c60f1716b13cf35fc899e2a36ea9c6cdc3478a8f051220eef0f53567efd
DEPS="glib libxfce4util dbus"
BUILDSYS=autotools
# GDBUS_CODEGEN is exported centrally by scripts/qnxbuild -- xfconf,
# xfce4-panel and tumbler all need it.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
