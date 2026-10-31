# libxfce4util 4.20.0

> Non-GUI utility library for the Xfce desktop

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://archive.xfce.org/src/xfce/libxfce4util/4.20/libxfce4util-4.20.0.tar.bz2 |
| SHA-256 | `21493f9c9995a282823db93839f6b9f06ae31edb094191ba9acf04d932a2b592` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`glib`](../glib/) |
| Run-time dependencies | [`gettext`](../gettext/) [`glib`](../glib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libxfce4util`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libxfce4util
apk add libxfce4util-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libxfce4util && /usr/local/bin/qnx-port-tests/test_libxfce4util    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libxfce4util`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libxfce4util` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (9 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild glib
```

<details><summary>Full build chain (9)</summary>

[`bzip2`](../bzip2/) [`gettext`](../gettext/) [`glib`](../glib/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`pcre2`](../pcre2/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
URL=https://archive.xfce.org/src/xfce/libxfce4util/4.20/libxfce4util-4.20.0.tar.bz2
mkdir -p dist src/libxfce4util
curl -fL -o dist/$(basename $URL) $URL
echo "21493f9c9995a282823db93839f6b9f06ae31edb094191ba9acf04d932a2b592  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libxfce4util --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libxfce4util < ports/libxfce4util/patches/0001-qnx-sa-restart.patch
```

**[`0001-qnx-sa-restart.patch`](patches/0001-qnx-sa-restart.patch)** — Define SA_RESTART as 0 on QNX, where <signal.h> leaves it commented out.

> QNX 8.0's <signal.h> carries SA_RESTART commented out ("not supported yet"):
> an interrupted kernel call returns EINTR rather than restarting.  This code
> only passes the flag to sigaction(); nothing branches on it, and the callers
> already retry on EINTR, so 0 is a true no-op rather than a behaviour change.

Touches: `libxfce4util/xfce-posix-signal-handler.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libxfce4util && cd build/$QNX_ARCH/libxfce4util
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/libxfce4util/configure --host=$QNX_TRIPLE --prefix=/usr/local \
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

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libxfce4util`.

### 5. What it installs

88 files under `/usr/local`: 2 shared-library entries, 1 programs, 17 headers.

```
/usr/local/lib/libxfce4util.so
/usr/local/lib/libxfce4util.so.7
/usr/local/sbin/xfce4-kiosk-query
```

The complete list is `stage/<arch>/.manifest/libxfce4util` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libxfce4util
ls dists/8.0/*/ | grep '^libxfce4util-'
```

This writes `libxfce4util-<ver>-r0.tar.gz` (run time) and `libxfce4util-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libxfce4util
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libxfce4util-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libxfce4util-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libxfce4util`](../../tests/src/) links libxfce4util the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libxfce4util`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libxfce4util
/usr/local/bin/qnx-port-tests/test_libxfce4util
```

```
== libxfce4util ==
  ok   ...
PASS libxfce4util
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libxfce4util` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libxfce4util`](../../tests/src/) -- a program linked against libxfce4util the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - version >= 4.16
   - xfce_mkdirhier a/b/c
   - xfce_rc_simple_open (read-write, new file)
   - rc dirty after writes
   - clean after flush
   - file on disk has [Panel] and size=48
   - reopen read-only
   - both groups present
   - string entry
   - int entry
   - bool entry
   - list entry (3 items)
   - default for a missing key
   - negative int in second group
   - parse a desktop file
   - untranslated Name + Exec
   - xfce_rc_get_entries
   - xfce_str_replace
   - xfce_utf8_remove_controls replaces controls with spaces
   - xfce_unescape_desktop_entry_value
   - xfce_expand_variables $VAR

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | libxfce4util 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/libxfce4util-4.20.0-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | libxfce4util 4.20.0-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/libxfce4util-4.20.0-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **4.20.0**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libxfce4util=<version>`), sources in `ported/libxfce4util/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libxfce4util`; port one with `scripts/port-update libxfce4util`.

<details><summary>recipe.sh</summary>

```bash
VERSION=4.20.0
# The release tarball, not the git tag: Xfce keeps its generated sources
# (*-alias.h, *-aliasdef.c, the .ui C sources) out of git and behind
# "if MAINTAINER_MODE", and regenerating them needs xdt-csource, which
# xfce4-dev-tools 4.20 no longer ships. The tarball has them all.
TARBALL=https://archive.xfce.org/src/xfce/libxfce4util/4.20/libxfce4util-4.20.0.tar.bz2
SHA256=21493f9c9995a282823db93839f6b9f06ae31edb094191ba9acf04d932a2b592
DEPS="glib"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --disable-gtk-doc
                  --disable-introspection --disable-linker-opts  )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
