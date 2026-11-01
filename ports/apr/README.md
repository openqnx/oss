# apr 1.7.6

> Apache Portable Runtime, a portable C API for system services

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://archive.apache.org/dist/apr/apr-1.7.6.tar.bz2 |
| SHA-256 | `49030d92d2575da735791b496dc322f3ce5cff9494779ba8cc28c7f46c5deb32` |
| Build system | GNU Autotools |
| QNX patches | 0 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_apr`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `apr` (programs, shared libraries, run-time data) and `apr-dev` (headers, pkg-config/CMake files, static libraries; needs `apr`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`apr`](https://openqnx.github.io/oss/package/x86_64/apr.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/apr.html)) · [`apr-dev`](https://openqnx.github.io/oss/package/x86_64/apr-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/apr-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add apr` (dependencies come along); one version: `apk add apr=<version>` | `apk del apr` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install apr` (dependencies first) | `oss-get remove apr` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`apr-dev` the same way, by its own name (`apk add apr-dev`, `oss-get install apr-dev`, by hand with its own file and list `apr-dev.list`); remove it before `apr`, which it needs.

```sh
# on the QNX target, as root
apk add apr apr-dev
apk add qnx-test-apr && /usr/local/bin/qnx-port-tests/test_apr    # its on-target test, from tests-8.0-<arch>
apk del apr apr-dev qnx-test-apr
```

By hand (B-2), from the release `packages-8.0-<arch>`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/apr.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/apr.list
rm var/lib/manual/apr.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild apr` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

No other component has to be built first.

### 2. Get the upstream source

```bash
URL=https://archive.apache.org/dist/apr/apr-1.7.6.tar.bz2
mkdir -p dist src/apr
curl -fL -o dist/$(basename $URL) $URL
echo "49030d92d2575da735791b496dc322f3ce5cff9494779ba8cc28c7f46c5deb32  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/apr --strip-components=1
```

### 3. Apply the QNX patches

None — the upstream source builds for QNX unmodified; everything QNX-specific is in the configuration below and in the shared toolchain files.

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/apr; B=build/$QNX_ARCH/apr; mkdir -p $B
```

```bash
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i -e 's|^APR_SOURCE_DIR=.*|APR_SOURCE_DIR=""|' -e 's|^APR_BUILD_DIR=.*|APR_BUILD_DIR=""|' "$STAGE$PREFIX/bin/apr-1-config"
}
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- configure runs ~30 test programs, none of which can run in a cross build. The answers, checked against the SDP 8.0 headers and QNX's behaviour: - /dev/zero exists and can be mapped shared; getaddrinfo/getnameinfo and AI_ADDRCONFIG work; EAI_* codes are positive (BSD numbering); - QNX's socket layer is FreeBSD's: an accepted socket inherits TCP_NODELAY and O_NONBLOCK from the listener (the run-level test checks the latter); there is no TCP_CORK (TCP_NOPUSH instead) and no SCTP; - accept4() and SOCK_CLOEXEC exist (SDP 8.0), dup3() does not; - epoll is only a Linux-compatibility shim (libepoll): APR uses poll(); - POSIX semaphores, process-shared and recursive pthread mutexes work; robust mutexes are left out of the default cross-process lock choice, which is then fcntl() locking, APR's classic default; - GCC's 32/64-bit __sync and __atomic builtins; strerror_r returns int; 64-bit off_t already (no _LARGEFILE64_SOURCE); struct rlimit. QNX's libc has no crypt(), and nothing in apr calls it: configure's search would otherwise link (and export to every user) libxcrypt. Likewise libuuid: apr_os_uuid_get() is used only when there is one, and APR's own UUIDs (random bytes and the clock) need none, so it is not looked for.
- The build files apr-util and others take from apr-1-config --installbuilddir (apr_rules.mk, libtool) go under share/, not upstream's $PREFIX/build-1.
- apr-1-config names the source and build trees for use from an uninstalled build; installed (or run from staging, "crosscompile") it never uses them, and a package must not name the build machine.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH apr`.

### 5. What it installs

52 files under `/usr/local`: 3 shared-library entries, 1 programs, 41 headers.

```
/usr/local/lib/libapr-1.so
/usr/local/lib/libapr-1.so.0
/usr/local/lib/libapr-1.so.0.7.6
/usr/local/bin/apr-1-config
```

The complete list is `stage/<arch>/.manifest/apr` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa apr
ls dists/8.0/*/ | grep '^apr-'
```

This writes `apr-<ver>-r0.tar.gz` (run time) and `apr-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add apr
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/apr-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/apr-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_apr`](../../tests/src/) links apr the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-apr`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-apr
/usr/local/bin/qnx-port-tests/test_apr
```

```
== apr ==
  ok   ...
PASS apr
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_apr` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_apr`](../../tests/src/) -- a program linked against apr the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - apr_initialize()
   - apr_pool_create()
   - apr_psprintf with APR_INT64_T_FMT
   - apr_strtok splits a,b,,c into 3 tokens
   - apr_strnatcmp: file10 > file9
   - apr_strfsize(1.5 MiB) = 1.5M
   - apr_table_get is case-insensitive
   - apr_table_merge joins with ', '
   - apr_hash set/get/count
   - apr_time_exp_gmt(1234567890) = Fri 2009-02-13 23:31:30
   - apr_time_exp_gmt_get round trip
   - apr_rfc822_date
   - apr_temp_dir_get
   - apr_dir_make_recursive
   - apr_file_open for writing
   - apr_stat: regular file of 18 bytes
   - apr_file_gets reads the second line
   - apr_file_seek + apr_file_read
   - apr_file_mktemp
   - apr_dir_read lists sub/ and data.txt with their types
   - 4 threads x 20000 mutex-protected increments = 80000
   - apr_atomic_inc32 from 4 threads = 80000
   - apr_atomic_cas32
   - apr_shm_create (anonymous)
   - apr_proc_mutex_create (default)
   - ... and 16 more

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | apr 1.7.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/apr-1.7.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | apr 1.7.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/apr-1.7.6-r0.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.7.6**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add apr=<version>`), sources in `ported/apr/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates apr`; port one with `scripts/port-update apr`.

<details><summary>recipe.sh</summary>

```bash
VERSION=1.7.6
TARBALL=https://archive.apache.org/dist/apr/apr-1.7.6.tar.bz2
SHA256=49030d92d2575da735791b496dc322f3ce5cff9494779ba8cc28c7f46c5deb32
BUILDSYS=autotools
# configure runs ~30 test programs, none of which can run in a cross build.
# The answers, checked against the SDP 8.0 headers and QNX's behaviour:
#  - /dev/zero exists and can be mapped shared; getaddrinfo/getnameinfo and
#    AI_ADDRCONFIG work; EAI_* codes are positive (BSD numbering);
#  - QNX's socket layer is FreeBSD's: an accepted socket inherits TCP_NODELAY
#    and O_NONBLOCK from the listener (the run-level test checks the latter);
#    there is no TCP_CORK (TCP_NOPUSH instead) and no SCTP;
#  - accept4() and SOCK_CLOEXEC exist (SDP 8.0), dup3() does not;
#  - epoll is only a Linux-compatibility shim (libepoll): APR uses poll();
#  - POSIX semaphores, process-shared and recursive pthread mutexes work;
#    robust mutexes are left out of the default cross-process lock choice,
#    which is then fcntl() locking, APR's classic default;
#  - GCC's 32/64-bit __sync and __atomic builtins; strerror_r returns int;
#    64-bit off_t already (no _LARGEFILE64_SOURCE); struct rlimit.
# QNX's libc has no crypt(), and nothing in apr calls it: configure's search
# would otherwise link (and export to every user) libxcrypt. Likewise libuuid:
# apr_os_uuid_get() is used only when there is one, and APR's own UUIDs
# (random bytes and the clock) need none, so it is not looked for.
CONF_ENV=( ac_cv_file__dev_zero=yes ac_cv_mmap__dev_zero=yes
  ac_cv_working_getaddrinfo=yes apr_cv_gai_addrconfig=yes ac_cv_working_getnameinfo=yes
  ac_cv_negative_eai=no ac_cv_tcp_nodelay_inherited=yes ac_cv_o_nonblock_inherited=yes
  apr_cv_tcp_nodelay_with_cork=no apr_cv_sctp=no ac_cv_ebcdic=no
  apr_cv_accept4=yes apr_cv_sock_cloexec=yes apr_cv_dup3=no
  apr_cv_epoll=no apr_cv_epoll_create1=no apr_cv_epoll_wait_has_reliable_timeout=no apr_cv_aio_msgq=no
  ac_cv_func_sem_open=yes apr_cv_process_shared_works=yes apr_cv_mutex_robust_shared=no
  apr_cv_mutex_recursive=yes
  ap_cv_atomic_builtins=yes ap_cv__atomic_builtins=yes ap_cv_atomic_builtins64=yes ap_cv__atomic_builtins64=yes
  ac_cv_strerror_r_rc_int=yes apr_cv_use_lfs64=no ac_cv_struct_rlimit=yes
  ac_cv_search_crypt=no ac_cv_search_uuid_create=no ac_cv_search_uuid_generate=no apr_cv_osuuid=no )
# The build files apr-util and others take from apr-1-config --installbuilddir
# (apr_rules.mk, libtool) go under share/, not upstream's $PREFIX/build-1.
CONF_ARGS_EXTRA=( --enable-shared --disable-static --with-installbuilddir="$PREFIX/share/apr-1/build" )
# apr-1-config names the source and build trees for use from an uninstalled
# build; installed (or run from staging, "crosscompile") it never uses them,
# and a package must not name the build machine.
do_install() {
  make -C "$B" DESTDIR="$STAGE" install &&
  sed -i -e 's|^APR_SOURCE_DIR=.*|APR_SOURCE_DIR=""|' -e 's|^APR_BUILD_DIR=.*|APR_BUILD_DIR=""|' "$STAGE$PREFIX/bin/apr-1-config"
}
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
