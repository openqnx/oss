# Porting an open source project to QNX SDP 8.0

How every component in `ports/` was made, step by step, so you can do the same
for a project that is not here yet. The worked examples are real ports from
this repository.

## Adding a project that is not here yet

```bash
scripts/new-port <name> <git-url | tarball-url> [<tag | commit>]
```

writes `ports/<name>/recipe.sh` (the newest release tag, or the tarball and
its SHA-256, and the build system it finds) and a test skeleton, and tries the
first build. The sections below are what follows: fix, patch, both
architectures, test. Then package (`scripts/qnxpkg --tests <name>`), generate
`ported/<name>/` (`scripts/gen-ported <name>`) and the docs, verify it on a
target (`scripts/verify-on-target`, [target guide](target-guide.md)), and
publish it on its own when it passed (`scripts/publish-github-release --only
<name> --upload --changed`).

A newer upstream version of a component already here is
`scripts/port-update <name> [<version>]` (`scripts/check-updates` lists them):
the version ported before keeps its recipe and patches in
`ports/<name>/versions/<old>/`, its packages in `dists/`, its sources in
`ported/<name>/<old>/` and its test results.

## The shape of a port

```
ports/<name>/
  recipe.sh          where the source comes from and how it is configured
  patches/
    0001-<what>.patch   one change per patch, each with a description
  README.md          generated from the two above -- never edited by hand
```

The source is never copied into this repository. A port is *upstream at a
pinned version + these patches + this configuration*.

## 1. Write the recipe

```bash
VERSION=1.9.0; GIT=https://github.com/nanomsg/nng.git; REF=v1.9.0
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DNNG_TESTS=OFF )
```

| Field | |
|---|---|
| `VERSION` | the upstream version |
| `GIT` + `REF` | clone this tag (preferred: reproducible and diffable) |
| `TARBALL` + `SHA256` | or download this release archive; the checksum is verified |
| `BUILDSYS` | `cmake` (default), `meson`, `autotools` or `custom` |
| `CMAKE_ARGS_EXTRA` / `MESON_ARGS_EXTRA` / `CONF_ARGS_EXTRA` | extra configure arguments |
| `CONF_ENV` | variables for an Autotools `configure` (cache answers, `LIBS=`) |
| `CFLAGS_APPEND` / `LDFLAGS_APPEND` | added to the target's `CFLAGS`/`CXXFLAGS` or `LDFLAGS` (Autotools); unlike `LIBS=`, never reaches a configure check of the build machine's own compiler |
| `COMMIT` | with `GIT`: fetch this exact commit (projects without release tags) |
| `CONFLICTS` | components that install the same files (two implementations of one library); packages carry it and `oss-get` refuses to install both. Apart from these, `qnxbuild` refuses an install that would overwrite a file another component installed. A component that builds and installs its own copies of shared libraries (TensorFlow Lite, PyTorch) goes in a prefix of its own, `$PREFIX/opt/<name>` |
| `ARCHES_SUPPORTED` | limit the component to these architectures, e.g. `"x86_64"` |
| `DEPS` | components that must be built first, e.g. `DEPS="openssl zlib"` |
| `SUBDIR` | the project lives in a subdirectory of the repository |
| `do_fetch`, `do_patch`, `do_configure`, `do_build`, `do_install` | shell functions that replace a default step |

Inside hooks, `$S` is the source directory, `$B` the build directory, `$STAGE`
the install root and `$PREFIX` is `/usr/local`. During `do_install`, `$STAGE`
is the component's own package root (that is how each package's file list is
recorded); for `-I`/`-L` paths into the staged dependencies use
`$STAGE_SYSROOT`, which always names the staging tree.

**Comment every non-obvious line.** The comments become the "why" section of
the component's README. Look at `ports/nng/recipe.sh` or
`ports/libmicrohttpd/recipe.sh`.

## 2. Build, read the first error, repeat

```bash
source ./env.sh aarch64le
./scripts/qnxbuild nng                 # log: logs/nng.aarch64le.log
```

Prefer, in this order:

1. **A configure option** — turn off what QNX cannot do (`--disable-epoll`),
   or pick a portable backend.
2. **A configure cache answer** — when a run-test cannot execute in a cross
   build (`CONF_ENV=( rsync_cv_MAKEDEV_TAKES_3_ARGS=yes )`).
3. **A patch** — when the source itself has to change.

### What usually breaks on QNX 8

| Symptom | Cause | Typical fix |
|---|---|---|
| `EPOLLRDHUP` / `EPOLLET` / `EPOLLONESHOT` undeclared | QNX's epoll shim implements the core API only | use the project's poll() backend (nng, libmicrohttpd), or define the flag to 0 where it is only advisory (libcoap) |
| `SA_RESTART` undeclared | commented out in `<signal.h>` ("not supported yet") | `#define SA_RESTART 0` where code handles `EINTR` (tmux, libevent) |
| `IOV_MAX` undeclared | `<limits.h>` undefines it | `UIO_MAXIOV` from `<sys/uio.h>` (tmux) |
| `makedev` takes 3 arguments | QNX's `makedev(node, major, minor)` | the project's 3-argument path (rsync) |
| `unknown type name 'inline'` in QNX headers | project builds as C90 | build as C99 on QNX (libgit2) |
| `sys/errno.h: No such file` | non-POSIX header | `<errno.h>` (nng) |
| `B230400` undeclared | termios stops at B115200 | `#ifdef` the case (libssh) |
| `undefined reference to regexec` | regex is in `libregex`, not libc | `LIBS=-lregex` (tmux) |
| `sighandler_t` unknown | glibc-only type | declare `void (*)(int)` (quickjs-ng) |
| `struct dirent` has no `d_type` | not in QNX | `stat()` instead, or skip the tool |
| `-dumpmachine` fails | `qcc` is a driver, not GCC | take `CMAKE_SYSTEM_PROCESSOR` (civetweb) |
| `-3s`, `-lunix` on the command line | the project still has QNX 4 (Watcom) flags | restrict them to `qnx4` hosts (bash) |
| `relocation ... recompile with -fPIC` (x86_64) | CMake OBJECT libraries without PIC | `-DCMAKE_POSITION_INDEPENDENT_CODE=ON` (open62541) |

[porting-hazards.md](porting-hazards.md) has ~100 more, each with the
component it was found in.

## 3. Turn the fix into a patch

Edit `src/<name>` directly until it builds, then:

```bash
./scripts/mkpatch nng posix-udp-errno-h "Include <errno.h>, not <sys/errno.h>." notes.txt
```

This rebuilds the pristine upstream tree, applies the existing patches, diffs
it against your edited tree and writes
`ports/nng/patches/0001-posix-udp-errno-h.patch` with the summary, the
description from `notes.txt` and an `Upstream-Status:` line. One concern per
patch. Then prove it applies to a fresh checkout:

```bash
rm -rf src/nng && ./scripts/qnxbuild -f nng
```

Patch descriptions say **why**: the QNX behaviour, and why the change is the
correct behaviour there — not only what the diff does.

## 4. Both architectures

```bash
./scripts/qnxbuild nng                 # no -a: aarch64le and x86_64
```

They fail differently. x86_64 is where missing `-fPIC` shows up; aarch64le is
where GCC 12 raises warnings that `-Werror` turns into errors.

## 5. Standalone or dependent?

```bash
./scripts/verify-standalone nng        # builds against an EMPTY staging tree
./scripts/gen-dependency-index         # classifies all components
```

A component that declares no `DEPS` must build with nothing else present.
If `gen-dependency-index` reports an *undeclared* run-time dependency, the
build picked up a library from staging that the recipe did not ask for —
either add it to `DEPS` or disable that feature. See
[DEPENDENCIES.md](../DEPENDENCIES.md) for the three classes.

## 6. A test on the target

Add `tests/src/test_<name>.c` (or `.cpp`): link the library as an application
would, exercise the core API, one `QT_CHECK` per behaviour — see
`tests/src/test_nng.c`. Register it in `tests/CMakeLists.txt`:

```cmake
port_test(test_nng   test_nng.c   nng)
```

Run it on a board or a `mkqnximage` VM:

```bash
./tests/build-tests.sh && ./tests/make-bundle.sh
./tests/deploy-and-run.sh aarch64le <target-ip>
```

A QEMU target for either architecture:

```bash
mkdir ~/vm-x86_64 && cd ~/vm-x86_64
mkqnximage --type=qemu --arch=x86_64 --data-size=3000 --ssh-ident=$HOME/.ssh/id_ed25519.pub
cp ~/oss/tests/vm/system_files.custom local/snippets/   # see below
mkqnximage --build && mkqnximage --run=-h
mkqnximage --getip                                      # changes on every boot
```

Every binary built here links QNX's Linux-compatibility libraries
(`libeventfd`, `libepoll`, `libsignalfd`, `libtimerfd`), and the audio ones
link `libasound`. A full QNX 8 image has them; mkqnximage's standard file list does not, and without them every
test fails at load with `Could not load library libeventfd.so.1`.
[`tests/vm/system_files.custom`](../tests/vm/system_files.custom) adds them
to the image's `/system/lib`, which is on the loader's search path. Log in as
`root` (`TARGET_USER=root`): with `--ssh-ident` only root accepts the key.

## 7. Package and document

```bash
./scripts/qnxpkg nng                   # dists/8.0/<arch>/nng-1.9.0-r0.{tar.gz,apk}
./scripts/gen-dependency-index && ./scripts/gen-port-docs && ./scripts/gen-root-readme
```
