# libedit 20240808-3.1

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://thrysoee.dk/editline/libedit-20240808-3.1.tar.gz |
| SHA-256 | `5f0573349d77c4a48967191cdd6634dd7aa5f6398c6a57fe037cc02696d6099f` |
| Build system | GNU Autotools |
| QNX patches | 1 |
| Build dependencies | [`ncurses`](../ncurses/) |
| Run-time dependencies | [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_libedit`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add libedit
apk add libedit-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-libedit && /usr/local/bin/qnx-port-tests/test_libedit    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install libedit`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild libedit` performs exactly the same steps (2–5) in one command.

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
URL=https://thrysoee.dk/editline/libedit-20240808-3.1.tar.gz
mkdir -p dist src/libedit
curl -fL -o dist/$(basename $URL) $URL
echo "5f0573349d77c4a48967191cdd6634dd7aa5f6398c6a57fe037cc02696d6099f  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/libedit --strip-components=1
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/libedit < ports/libedit/patches/0001-qnx-no-sys-ttydefaults.patch
```

**[`0001-qnx-no-sys-ttydefaults.patch`](patches/0001-qnx-no-sys-ttydefaults.patch)** — libedit: no <sys/ttydefaults.h> on QNX.

> Take the existing fallback CTRL() definition, which is what that header
> would have provided.

Touches: `src/editline/readline.h` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

```bash
mkdir -p build/$QNX_ARCH/libedit && cd build/$QNX_ARCH/libedit
CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CXX="q++ -V${QCC_TARGET}_cxx -D_QNX_SOURCE" \
CPP="$QNX_HOST/usr/bin/$QNX_TRIPLE-gcc -E" \
CPPFLAGS="-D_QNX_SOURCE -I$STAGE/usr/local/include -I$QNX_TARGET/usr/include" \
CFLAGS="$CPPFLAGS" CXXFLAGS="$CPPFLAGS" \
LDFLAGS="-L$STAGE/usr/local/lib -Wl,-rpath-link,$STAGE/usr/local/lib \
         -lsocket -lm -leventfd -lepoll -lsignalfd -ltimerfd -lfsnotify" \
LIBS="-lregex" \
$OLDPWD/src/libedit/configure --host=$QNX_TRIPLE --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --enable-widec
make -j$(nproc)
make DESTDIR=$STAGE install
cd -
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- libedit's history search uses POSIX regex, which QNX keeps in its own libregex rather than in libc. Without this the shared library is linked with regcomp/regexec/regfree unresolved and every consumer fails to link.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH libedit`.

### 5. What it installs

48 files under `/usr/local`: 2 shared-library entries, 2 headers.

```
/usr/local/lib/libedit.so
/usr/local/lib/libedit.so.0
```

The complete list is `stage/<arch>/.manifest/libedit` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa libedit
ls dists/8.0/*/ | grep '^libedit-'
```

This writes `libedit-<ver>-r0.tar.gz` (run time) and `libedit-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add libedit
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/libedit-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/libedit-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_libedit`](../../tests/src/) links libedit the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-libedit`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-libedit
/usr/local/bin/qnx-port-tests/test_libedit
```

```
== libedit ==
  ok   ...
PASS libedit
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_libedit` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_libedit`](../../tests/src/) -- a program linked against libedit the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - history_init
   - three entries stored
   - H_FIRST returns the newest entry
   - H_LAST returns the oldest entry
   - H_PREV_STR (regex search)
   - tok_init
   - tok_str
   - four tokens
   - quoting is honoured

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date |
|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 |

## Versions

This recipe builds **20240808-3.1**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add libedit=<version>`), sources in `ported/libedit/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates libedit`; port one with `scripts/port-update libedit`.

<details><summary>recipe.sh</summary>

```bash
VERSION=20240808-3.1
TARBALL=https://thrysoee.dk/editline/libedit-20240808-3.1.tar.gz
SHA256=5f0573349d77c4a48967191cdd6634dd7aa5f6398c6a57fe037cc02696d6099f
DEPS="ncurses"
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --enable-shared --disable-static --enable-widec )
# libedit's history search uses POSIX regex, which QNX keeps in its own
# libregex rather than in libc. Without this the shared library is linked with
# regcomp/regexec/regfree unresolved and every consumer fails to link.
CONF_ENV=( LIBS="-lregex" )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
