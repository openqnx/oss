# picocom 3.1

**🟢 Standalone** — Builds with only the QNX SDP and runs on a stock QNX 8.0 target. Nothing else from this repository is needed, at build time or at run time.

| | |
|---|---|
| Upstream | https://github.com/npat-efault/picocom.git (tag `3.1`) |
| Build system | custom (see recipe) |
| QNX patches | 1 |
| Build dependencies | none |
| Run-time dependencies | none (QNX SDP only) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | none (no run-time code) |

## Install the prebuilt package on a target

No build needed: [`oss-get`](../../target/oss-get) downloads the package (and its run-time dependencies), checks its SHA-256 and unpacks it.

```sh
# on the QNX target
oss-get update
oss-get install picocom
oss-get install picocom-dev    # only for building on the target: headers, .pc, CMake files
```

See [docs/packages.md](../../docs/packages.md) for setting up `oss-get` on a board.

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild picocom` performs exactly the same steps (2–5) in one command.

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
git clone --depth 1 -b 3.1 --recurse-submodules --shallow-submodules \
    https://github.com/npat-efault/picocom.git src/picocom
```

### 3. Apply the QNX patches

1 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/picocom < ports/picocom/patches/0001-qnx-select-and-hw-flow-control.patch
```

**[`0001-qnx-select-and-hw-flow-control.patch`](patches/0001-qnx-select-and-hw-flow-control.patch)** — Build on QNX: select() headers, ioctl, RTS/CTS flow control.

> - picocom.c uses select(), fd_set and struct timeval without including
>   <sys/select.h>/<sys/time.h>, which glibc pulls in through other headers
>   and QNX does not.
> - term.c only uses the modem-control ioctls (TIOCMGET/TIOCMSET) on a list of
>   known systems; QNX supports them too.
> - QNX has no CRTSCTS: hardware flow control is the pair of c_cflag bits
>   IHFLOW | OHFLOW. Defining CRTSCTS as that pair keeps "--flow h" working
>   (defining it as 0, as other QNX ports do, silently disables it).
>
> Based on qnx-ports/build-files ports/picocom/0001-qnx-compat.patch.

Touches: `picocom.c`, `term.c` · Upstream status: QNX-specific

### 4. Configure, build and install into the staging tree

This component uses its own build steps (`do_build`, `do_install`), run with `S` = the source tree, `B` = the build directory:

```bash
S=src/picocom; B=build/$QNX_ARCH/picocom; mkdir -p $B
```

```bash
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CPPFLAGS="-D_QNX_SOURCE -DVERSION_STR=\\\"$VERSION\\\" -DHIGH_BAUD" LDFLAGS=""; }
```

```bash
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/picocom" "$STAGE$PREFIX/bin/"; }
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH picocom`.

### 5. What it installs

1 files under `/usr/local`, 1 programs.

```
/usr/local/bin/picocom
```

The complete list is `stage/<arch>/.manifest/picocom` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH picocom
ls repo/8.0/*/ | grep '^picocom-'
```

This writes `picocom-<ver>-r0.tar.gz` (run time) and `picocom-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Either install from your own repository (serve `repo/8.0/` with any web server):

```sh
# on the target
export OSS_REPO=http://<build-host>:8000
oss-get update && oss-get install picocom
```

or copy the tarball and unpack it yourself:

```bash
scp repo/8.0/aarch64/picocom-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'cd /data/oss && tar xzf /data/picocom-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `export LD_LIBRARY_PATH=/usr/local/lib` (with `oss-get link`) or `eval $(oss-get env)`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.1; GIT=https://github.com/npat-efault/picocom.git; REF=3.1
BUILDSYS=custom
do_build() { cp -a "$S/." "$B/" && make -C "$B" CC="qcc -V$QCC_TARGET" CPPFLAGS="-D_QNX_SOURCE -DVERSION_STR=\\\"$VERSION\\\" -DHIGH_BAUD" LDFLAGS=""; }
do_install() { install -d "$STAGE$PREFIX/bin" && install -m755 "$B/picocom" "$STAGE$PREFIX/bin/"; }
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
