# readline 8.2

> GNU Readline library for command line editing and history

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

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add readline
apk add readline-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-readline && /usr/local/bin/qnx-port-tests/test_readline    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install readline`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

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
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa readline
ls dists/8.0/*/ | grep '^readline-'
```

This writes `readline-<ver>-r0.tar.gz` (run time) and `readline-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add readline
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/readline-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/readline-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_readline`](../../tests/src/) links readline the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-readline`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-readline
/usr/local/bin/qnx-port-tests/test_readline
```

```
== readline ==
  ok   ...
PASS readline
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_readline` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_readline`](../../tests/src/) -- a program linked against readline the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - rl_readline_version
   - history_list
   - three entries in the history
   - first entry
   - history_search_prefix found a match
   - remove_history
   - history_expand(\"!!\")
   - rl_line_buffer is allocated
   - rl_insert_text
   - rl_point tracks the insertion
   - rl_delete_text cleared the buffer
   - clear_history

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | readline 8.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/readline-8.2-r0.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | readline 8.2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/readline-8.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **8.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add readline=<version>`), sources in `ported/readline/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates readline`; port one with `scripts/port-update readline`.

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
