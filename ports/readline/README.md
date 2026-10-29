# readline 8.2

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz |
| SHA-256 | `3feb7171f16a84ee82ca18a36d7b9be109a52c04f492a053331d7d1095007c35` |
| Build system | GNU Autotools |
| QNX patches | 2 |
| Build dependencies | [`ncurses`](../ncurses/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_readline`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install readline
oss-get install readline-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild readline` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild ncurses
```

### 2. Get the upstream source

```bash
URL=https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz
mkdir -p dist src/readline
curl -fL -o dist/$(basename $URL) $URL
echo "3feb7171f16a84ee82ca18a36d7b9be109a52c04f492a053331d7d1095007c35  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/readline --strip-components=1
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/readline < ports/readline/patches/0001-qnx-shlib-install-symlinks.patch
patch -p1 -d src/readline < ports/readline/patches/0002-qnx-shobj-conf.patch
```

**[`0001-qnx-shlib-install-symlinks.patch`](patches/0001-qnx-shlib-install-symlinks.patch)** — Install the shared-library symlinks on QNX.

> support/shlib-install knows how to make libreadline.so.M and libreadline.so
> only for the systems it lists; for QNX it makes none, and "-lreadline" then
> finds no library. Add QNX to the chmod case and a symlink case that copes
> with a two-component library name (no LINK2).

Touches: `support/shlib-install` · Upstream status: QNX-specific

**[`0002-qnx-shobj-conf.patch`](patches/0002-qnx-shobj-conf.patch)** — Build QNX shared libraries with an soname.

> support/shobj-conf has no case for QNX (nto-qnx*), so it reports shared
> objects as unsupported and the libraries come out without DT_SONAME:
> programs linked against them record libreadline.so -- the -dev symlink --
> and fail to start where only the run-time package is installed. Add a case
> with the Linux rules (lib<name>.so.<major> as the soname), minus the rpath
> and minus the second -soname the Linux case puts in SHOBJ_LDFLAGS.

Touches: `support/shobj-conf` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/readline && cd build/$QNX_ARCH/readline
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
$OLDPWD/src/readline/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --with-curses
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH readline`.

### 5. What it installs

24 files under `/usr/local`: 6 shared-library entries, 8 headers.

```
/usr/local/lib/libhistory.so
/usr/local/lib/libhistory.so.8
/usr/local/lib/libhistory.so.8.2
/usr/local/lib/libreadline.so
/usr/local/lib/libreadline.so.8
/usr/local/lib/libreadline.so.8.2
```

The complete list is `stage/<arch>/.manifest/readline` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH readline
ls repo/8.0/*/ | grep '^readline-'
```

This writes `readline-<ver>-r0.tar.gz` (run time) and `readline-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install readline
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/readline-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/readline-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

### Test

[`test_readline`](../../tests/src/) links readline the way an application would and checks its core API on the target:

```bash
./tests/build-tests.sh $QNX_ARCH
scp stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_readline root@<target>:/data/
ssh root@<target> 'LD_LIBRARY_PATH=/usr/local/lib /data/test_readline'
```

```
== readline ==
  ok   ...
PASS readline
```

`./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

<details><summary>recipe.sh</summary>

```bash
VERSION=8.2
TARBALL=https://ftp.gnu.org/gnu/readline/readline-8.2.tar.gz
SHA256=3feb7171f16a84ee82ca18a36d7b9be109a52c04f492a053331d7d1095007c35
BUILDSYS=autotools
DEPS="ncurses"
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-curses )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
