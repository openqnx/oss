# sudo 1.9.17p2

> Runs commands with the privileges of another user

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://www.sudo.ws/dist/sudo-1.9.17p2.tar.gz |
| SHA-256 | `4a38a1ab3adb1199257edc2a7c4a2bd714665eb605b04368843b06dada2cfcfb` |
| Build system | GNU Autotools |
| QNX patches | 3 |
| Build dependencies | [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Run-time dependencies | [`libxcrypt`](../libxcrypt/) [`openssl`](../openssl/) [`zlib`](../zlib/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_sudo`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `sudo` (programs, shared libraries, run-time data) and `sudo-dev` (headers, pkg-config/CMake files, static libraries; needs `sudo`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`sudo`](https://openqnx.github.io/oss/package/x86_64/sudo.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sudo.html)) · [`sudo-dev`](https://openqnx.github.io/oss/package/x86_64/sudo-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/sudo-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add sudo` (dependencies come along); one version: `apk add sudo=<version>` | `apk del sudo` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install sudo` (dependencies first) | `oss-get remove sudo` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`sudo-dev` the same way, by its own name (`apk add sudo-dev`, `oss-get install sudo-dev`, by hand with its own file and list `sudo-dev.list`); remove it before `sudo`, which it needs.

```sh
# on the QNX target, as root
apk add sudo sudo-dev
apk add qnx-test-sudo && /usr/local/bin/qnx-port-tests/test_sudo    # its on-target test, from tests-8.0-<arch>-m-z
apk del sudo sudo-dev qnx-test-sudo
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/sudo.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/sudo.list
rm var/lib/manual/sudo.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild sudo` performs exactly the same steps (2–5) in one command.

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
./scripts/qnxbuild libxcrypt openssl zlib
```

### 2. Get the upstream source

```bash
URL=https://www.sudo.ws/dist/sudo-1.9.17p2.tar.gz
mkdir -p dist src/sudo
curl -fL -o dist/$(basename $URL) $URL
echo "4a38a1ab3adb1199257edc2a7c4a2bd714665eb605b04368843b06dada2cfcfb  dist/$(basename $URL)" | sha256sum -c -
tar xf dist/$(basename $URL) -C src/sudo --strip-components=1
```

### 3. Apply the QNX patches

3 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/sudo < ports/sudo/patches/0001-export-compat-usershell.patch
patch -p1 -d src/sudo < ports/sudo/patches/0002-qnx-pututline-void.patch
patch -p1 -d src/sudo < ports/sudo/patches/0003-qnx-no-rlimit-fsize.patch
```

**[`0001-export-compat-usershell.patch`](patches/0001-export-compat-usershell.patch)** — Export all three compat getusershell functions from libsudo_util.

> On a system without getusershell() (QNX), sudo builds its own
> setusershell/getusershell/endusershell into libsudo_util as sudo_*, but
> configure adds only sudo_getusershell to the library's export list. The
> sudoers plugin and visudo call the other two, so linking them fails with
> "undefined reference to sudo_setusershell" / "sudo_endusershell".
>
> configure is changed along with configure.ac so the release tarball builds
> without regenerating it.

Touches: `configure.ac`, `configure` · Upstream status: Pending (affects any system without getusershell())

**[`0002-qnx-pututline-void.patch`](patches/0002-qnx-pututline-void.patch)** — Cope with QNX's void pututline().

> sudo checks pututline()'s return value (the written entry, or NULL), but
> QNX declares it as returning void: "void value not ignored as it ought to
> be" in src/utmp.c. Treat the call as successful and yield the entry.

Touches: `src/utmp.c` · Upstream status: QNX-specific

**[`0003-qnx-no-rlimit-fsize.patch`](patches/0003-qnx-no-rlimit-fsize.patch)** — QNX rejects setrlimit(RLIMIT_FSIZE): do not try to raise it

Touches: `src/limits.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/sudo; B=build/$QNX_ARCH/sudo; mkdir -p $B
```

```bash
do_install() { make -C "$B" DESTDIR="$STAGE" INSTALL_OWNER= install; }
```

Why the configuration looks like this (from [`recipe.sh`](recipe.sh)):

- sudo. Version as in the QNX Everywhere 8.0.4r1 image.
- Also linked at run time (found by scripts/gen-dependency-index): libxcrypt openssl zlib
- QNX's <signal.h> has SA_RESTART commented out ("not supported yet"); sudo only asks for it as a convenience.
- QNX's libc has dl_iterate_phdr() but no <link.h>; sudo only uses it as one more entropy source.
- libsudo_util uses regcomp()/regexec(), which QNX keeps in libregex. Not via LIBS: configure also links its build-machine compiler test with LIBS, and the build machine has no libregex.
- sysconfdir under the prefix: packages install into the apk root (/data/oss), so a sudoers in /etc would never be the one shipped.
- sudo's install chowns its files to root, which an unprivileged build cannot do ("Operation not permitted"). Install without the owner flags: the modes, setuid included, are kept, and scripts/qnxpkg records every file as root.

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH sudo`.

### 5. What it installs

51 files under `/usr/local`, 7 programs, 1 headers.

```
/usr/local/bin/cvtsudoers
/usr/local/bin/sudo
/usr/local/bin/sudoedit
/usr/local/bin/sudoreplay
/usr/local/sbin/sudo_logsrvd
/usr/local/sbin/sudo_sendlog
/usr/local/sbin/visudo
```

The complete list is `stage/<arch>/.manifest/sudo` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa sudo
ls dists/8.0/*/ | grep '^sudo-'
```

This writes `sudo-<ver>-r0.tar.gz` (run time) and `sudo-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add sudo
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/sudo-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/sudo-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_sudo`](../../tests/src/) links sudo the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-sudo`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-sudo
/usr/local/bin/qnx-port-tests/test_sudo
```

```
== sudo ==
  ok   ...
PASS sudo
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_sudo` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_sudo`](../../tests/src/) -- a program linked against sudo the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:


Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sudo 1.9.17_p2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/aarch64le/sudo-1.9.17_p2-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | sudo 1.9.17_p2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/sudo-1.9.17_p2-r1.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sudo 1.9.17_p2-r0 | ✅ PASS | 2026-10-31 | [log](../../tests/logs/x86_64/sudo-1.9.17_p2-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | sudo 1.9.17_p2-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/sudo-1.9.17_p2-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **1.9.17p2**. Every version stays available: packages in `dists/8.0/<arch>/` (`apk add sudo=<version>`), sources in `ported/sudo/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates sudo`; port one with `scripts/port-update sudo`.

<details><summary>recipe.sh</summary>

```bash
# sudo. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=1.9.17p2
# Also linked at run time (found by scripts/gen-dependency-index): libxcrypt openssl zlib
DEPS="libxcrypt openssl zlib"
TARBALL=https://www.sudo.ws/dist/sudo-1.9.17p2.tar.gz
SHA256=4a38a1ab3adb1199257edc2a7c4a2bd714665eb605b04368843b06dada2cfcfb
BUILDSYS=autotools
# QNX's <signal.h> has SA_RESTART commented out ("not supported yet"); sudo
# only asks for it as a convenience.
CFLAGS_APPEND="-DSA_RESTART=0"
# QNX's libc has dl_iterate_phdr() but no <link.h>; sudo only uses it as one
# more entropy source.
CONF_ENV=( ac_cv_func_dl_iterate_phdr=no )
# libsudo_util uses regcomp()/regexec(), which QNX keeps in libregex. Not via
# LIBS: configure also links its build-machine compiler test with LIBS, and
# the build machine has no libregex.
LDFLAGS_APPEND="-lregex"
# sysconfdir under the prefix: packages install into the apk root
# (/data/oss), so a sudoers in /etc would never be the one shipped.
CONF_ARGS_EXTRA=( --disable-nls --without-pam --with-env-editor --sysconfdir="$PREFIX/etc" )
# sudo's install chowns its files to root, which an unprivileged build cannot
# do ("Operation not permitted"). Install without the owner flags: the modes,
# setuid included, are kept, and scripts/qnxpkg records every file as root.
do_install() { make -C "$B" DESTDIR="$STAGE" INSTALL_OWNER= install; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
