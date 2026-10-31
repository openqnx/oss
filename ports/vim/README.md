# vim 9.1.0900

> Vi IMproved, a highly configurable text editor

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/vim/vim.git (tag `v9.1.0900`) |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | [`ncurses`](../ncurses/) [`libiconv`](../libiconv/) [`libsodium`](../libsodium/) |
| Run-time dependencies | [`libiconv`](../libiconv/) [`libsodium`](../libsodium/) [`ncurses`](../ncurses/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vim`](../../tests/src/) |

## Install the prebuilt package on a target

No build needed. With `apk` (set up once per target with [`scripts/target-bootstrap`](../../scripts/target-bootstrap)), the package and its run-time dependencies are downloaded, signature-checked and installed:

```sh
# on the QNX target
. /data/oss/etc/oss-env.sh
apk add vim
apk add vim-dev    # only for building on the target: headers, .pc, CMake files
apk add qnx-test-vim && /usr/local/bin/qnx-port-tests/test_vim    # its on-target test
```

Or with [`oss-get`](../../target/oss-get), a single shell script: `oss-get install vim`. Both read the GitHub releases by default, or your own build served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)); see [docs/packages.md](../../docs/packages.md).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vim` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (3 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild ncurses libiconv libsodium
```

### 2. Get the upstream source

```bash
git clone --depth 1 -b v9.1.0900 --recurse-submodules --shallow-submodules \
    https://github.com/vim/vim.git src/vim
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/vim; B=build/$QNX_ARCH/vim; mkdir -p $B
```

```bash
do_configure() { cd "$B" && cp -a "$SRCROOT/." "$B/" && cd "$B" && \
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPPFLAGS="-I$STAGE$PREFIX/include -I$STAGE$PREFIX/include/ncursesw" \
      LDFLAGS="-L$STAGE$PREFIX/lib" LIBS="-lsocket" "${CONF_ENV[@]}" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}"; }
```

```bash
do_build() { make -C "$B" -j"$(nproc)"; }
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" STRIP="$QNX_HOST/usr/bin/nto${QNX_CPU}-strip" install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- Also linked at run time (found by scripts/gen-dependency-index): libiconv libsodium
- vim's configure probes several things by running test programs; these are the answers for QNX, which configure cannot find out itself when cross compiling. vim calls socket functions (for its server/clipboard code) that live in QNX's libsocket; configure does not know to link it.
- vim strips the binary it installs; with the host strip that fails on the other architecture ("Unable to recognise the format").

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vim`.

### 5. What it installs

1955 files under `/usr/local`, 8 programs.

```
/usr/local/bin/ex
/usr/local/bin/rview
/usr/local/bin/rvim
/usr/local/bin/view
/usr/local/bin/vim
/usr/local/bin/vimdiff
/usr/local/bin/vimtutor
/usr/local/bin/xxd
```

The complete list is `stage/<arch>/.manifest/vim` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vim
ls dists/8.0/*/ | grep '^vim-'
```

This writes `vim-<ver>-r0.tar.gz` (run time) and `vim-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vim
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vim-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vim-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vim`](../../tests/src/) links vim the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vim`, in the tests repository (`dists/8.0/tests/`, `scripts/qnxpkg --tests`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vim
/usr/local/bin/qnx-port-tests/test_vim
```

```
== vim ==
  ok   ...
PASS vim
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vim` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vim`](../../tests/src/) -- a program linked against vim the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vim 9.1.0900-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/vim-9.1.0900-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vim 9.1.0900-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/vim-9.1.0900-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **9.1.0900**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vim=<version>`), sources in `ported/vim/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vim`; port one with `scripts/port-update vim`.

<details><summary>recipe.sh</summary>

```bash
VERSION=9.1.0900; GIT=https://github.com/vim/vim.git; REF=v9.1.0900
# Also linked at run time (found by scripts/gen-dependency-index): libiconv libsodium
DEPS="ncurses libiconv libsodium"
BUILDSYS=autotools
# vim's configure probes several things by running test programs; these are the
# answers for QNX, which configure cannot find out itself when cross compiling.
# vim calls socket functions (for its server/clipboard code) that live in
# QNX's libsocket; configure does not know to link it.
CONF_ENV=( vim_cv_toupper_broken=no vim_cv_terminfo=yes vim_cv_tgetent=zero vim_cv_getcwd_broken=no
           vim_cv_stat_ignores_slash=no vim_cv_memmove_handles_overlap=yes vim_cv_bcopy_handles_overlap=yes
           vim_cv_memcpy_handles_overlap=no ac_cv_small_wchar_t=no vim_cv_uname_output=QNX vim_cv_uname_r_output=8.0.0
           vim_cv_uname_m_output=unknown )
CONF_ARGS_EXTRA=( --with-features=huge --enable-gui=no --without-x --disable-nls --with-tlib=ncursesw
                  --disable-netbeans --disable-channel --enable-multibyte )
SUBDIR=
do_configure() { cd "$B" && cp -a "$SRCROOT/." "$B/" && cd "$B" && \
  env CC="qcc -V$QCC_TARGET -D_QNX_SOURCE" CPPFLAGS="-I$STAGE$PREFIX/include -I$STAGE$PREFIX/include/ncursesw" \
      LDFLAGS="-L$STAGE$PREFIX/lib" LIBS="-lsocket" "${CONF_ENV[@]}" ./configure --host="$QNX_TRIPLE" --prefix="$PREFIX" "${CONF_ARGS_EXTRA[@]}"; }
do_build() { make -C "$B" -j"$(nproc)"; }
# vim strips the binary it installs; with the host strip that fails on the
# other architecture ("Unable to recognise the format").
do_install() { make -C "$B" DESTDIR="$STAGE" STRIP="$QNX_HOST/usr/bin/nto${QNX_CPU}-strip" install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
