# psutil 7.2.2

> Cross-platform process and system utilities module for Python

**🟡 Runtime-standalone** — Needs other components **to build** (listed below), but what it installs runs on the target **on its own**.

| | |
|---|---|
| Upstream | https://github.com/giampaolo/psutil.git (tag `release-7.2.2`) |
| Build system | custom (see recipe) |
| QNX patches | 2 |
| Build dependencies | [`python3`](../python3/) |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_psutil`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Package: `psutil`.

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`psutil`](https://openqnx.github.io/oss/package/x86_64/psutil.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/psutil.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add psutil` (dependencies come along); one version: `apk add psutil=<version>` | `apk del psutil` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install psutil` (dependencies first) | `oss-get remove psutil` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

```sh
# on the QNX target, as root
apk add qnx-test-psutil && /usr/local/bin/qnx-port-tests/test_psutil    # its on-target test, from tests-8.0-<arch>-m-z
apk del psutil qnx-test-psutil
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/psutil.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/psutil.list
rm var/lib/manual/psutil.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild psutil` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (15 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild python3
```

<details><summary>Full build chain (15)</summary>

[`bzip2`](../bzip2/) [`expat`](../expat/) [`gdbm`](../gdbm/) [`gettext`](../gettext/) [`libffi`](../libffi/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`openssl`](../openssl/) [`python3`](../python3/) [`readline`](../readline/) [`sqlite`](../sqlite/) [`xz`](../xz/) [`zlib`](../zlib/) [`zstd`](../zstd/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b release-7.2.2 --recurse-submodules --shallow-submodules \
    https://github.com/giampaolo/psutil.git src/psutil
```

### 3. Apply the QNX patches

2 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/psutil < ports/psutil/patches/0001-qnx-platform-from-qnx-ports.patch
patch -p1 -d src/psutil < ports/psutil/patches/0002-qnx-fixes.patch
```

**[`0001-qnx-platform-from-qnx-ports.patch`](patches/0001-qnx-platform-from-qnx-ports.patch)** — QNX platform module for psutil, from qnx-ports

> Imported from github.com/qnx-ports/psutil, branch qnx-7.2.2 (commit
> 00b09b8, release-7.2.2...qnx-7.2.2), restricted to the QNX changes: the
> _psutil_qnx extension (psutil/_psutil_qnx.c, psutil/arch/qnx/: CPU times
> and frequency from the system page, processes through /proc/<pid>/as
> devctls, network interfaces and their stats), the Python side
> (psutil/_psqnx.py, QNX in _common/__init__/_ntuples) and the QNX branches
> in the shared POSIX code and setup.py. The upstream-formatting changes the
> branch also carries for other platforms are left out.

Touches: `psutil/__init__.py`, `psutil/_common.py`, `psutil/_ntuples.py`, `psutil/_psqnx.py`, `psutil/_psutil_qnx.c`, `psutil/arch/all/init.h` … · Upstream status: QNX-specific (carried in the qnx-ports fork)

**[`0002-qnx-fixes.patch`](patches/0002-qnx-fixes.patch)** — Fix the QNX module: cross build, Process(), cpu_times

> Make the qnx-ports QNX module usable: as imported (0001), psutil.Process()
> raised AttributeError for every process (create_time() called
> self.self.proc_basic_info), and several other entry points could not work:
>
>  - setup.py chooses the platform from sys.platform, which in a cross build
>    is the build machine's: Linux's extension was compiled. Take it from
>    _PYTHON_HOST_PLATFORM when that names QNX.
>  - _common.__all__ lacked a comma after 'QNX', which glued it to 'SUNOS'.
>  - create_time(), num_threads(), cpu_times() and ppid() used wrong method
>    or field names; debug_process_t's times are nanoseconds, not clock ticks.
>  - gids() built a puids tuple; name() returned "UNIMPLEMENTED" (now the
>    executable's base name, from /proc/<pid>/exefile); cmdline() returned
>    one string with NULs inside instead of the argument list.
>  - A process that does not exist was reported as AccessDenied (every open
>    failure became EACCES) and leaked the descriptor on devctl errors; it is
>    now NoSuchProcess (ENOENT/ESRCH). wrap_exceptions() called
>    cext.proc_is_zombie, which only BSD and macOS have.
>  - System cpu_times() returned None, so cpu_percent() failed. QNX keeps no
>    per-CPU user/system split; it has the run time of procnto's idle
>    threads (process 1, one per CPU), which QNX's own hogs/top use: idle is
>    that, busy ("system") the time since boot less it.
>  - cpu_count(logical=False) returned None (QNX has no SMT notion: cores =
>    CPUs), disk_partitions() None (now []), and disk_usage was missing
>    (the POSIX statvfs implementation).
>  - proc_threads() leaked a reference per thread.

Touches: `psutil/arch/qnx/proc.c`, `psutil/_common.py`, `psutil/_ntuples.py`, `psutil/_psqnx.py`, `setup.py` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_configure`, `do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/psutil; B=build/$QNX_ARCH/psutil; mkdir -p $B
```

```bash
do_configure() { qnx_py_venv "setuptools>=43"; }
```

```bash
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
```

```bash
do_install() { qnx_py_install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- psutil: process and system utilities for Python, with the QNX platform module from github.com/qnx-ports/psutil (patches/0001: _psutil_qnx and _psqnx.py -- CPU count and frequency, memory from /proc/vm/stats, network interfaces, processes through /proc/<pid>/as) plus fixes to it (0002-).
- A setuptools C extension, cross compiled: setup.py picks the platform module from sys.platform, which is the build machine's -- patches/0002 takes it from _PYTHON_HOST_PLATFORM (set by qnx_py_wheel) instead. The target's Python headers come first on the include path (the venv's own are the build machine's, whose pyconfig.h is Linux's).

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH psutil`.

### 5. What it installs

17 files under `/usr/local`.

The complete list is `stage/<arch>/.manifest/psutil` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa psutil
ls dists/8.0/*/ | grep '^psutil-'
```

This writes `psutil-<ver>-r0.tar.gz` (run time) and `psutil-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add psutil
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/psutil-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/psutil-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_psutil`](../../tests/src/) links psutil the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-psutil`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-psutil
/usr/local/bin/qnx-port-tests/test_psutil
```

```
== psutil ==
  ok   ...
PASS psutil
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_psutil` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_psutil`](../../tests/src/) -- a program linked against psutil the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | psutil 7.2.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/psutil-7.2.2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | psutil 7.2.2-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/psutil-7.2.2-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **7.2.2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add psutil=<version>`), sources in `ported/psutil/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates psutil`; port one with `scripts/port-update psutil`.

<details><summary>recipe.sh</summary>

```bash
# psutil: process and system utilities for Python, with the QNX platform
# module from github.com/qnx-ports/psutil (patches/0001: _psutil_qnx and
# _psqnx.py -- CPU count and frequency, memory from /proc/vm/stats, network
# interfaces, processes through /proc/<pid>/as) plus fixes to it (0002-).
VERSION=7.2.2; GIT=https://github.com/giampaolo/psutil.git; REF=release-7.2.2
DEPS="python3"
RUNTIME_DEPS="python3"
BUILDSYS=custom
source "$WS/scripts/lib/pywheel.sh"
do_configure() { qnx_py_venv "setuptools>=43"; }
# A setuptools C extension, cross compiled: setup.py picks the platform
# module from sys.platform, which is the build machine's -- patches/0002
# takes it from _PYTHON_HOST_PLATFORM (set by qnx_py_wheel) instead. The
# target's Python headers come first on the include path (the venv's own
# are the build machine's, whose pyconfig.h is Linux's).
do_build() {
  rm -rf "$S/build"   # setuptools' build tree is in the source: no stale objects
  CFLAGS="-O2 -D_QNX_SOURCE -I$STAGE$PREFIX/include/python$QNX_PYV" qnx_py_wheel
}
do_install() { qnx_py_install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
