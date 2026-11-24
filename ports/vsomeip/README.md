# vsomeip 3.8.0

> COVESA SOME/IP protocol stack for automotive communication

**🔴 Dependent** — Needs other components **to build and at run time**. `oss-get` installs the run-time ones with it.

| | |
|---|---|
| Upstream | https://github.com/COVESA/vsomeip.git (tag `3.8.0`) |
| Build system | CMake |
| QNX patches | 5 |
| Build dependencies | [`boost`](../boost/) |
| Run-time dependencies | [`boost`](../boost/) |
| Targets | QNX SDP 8.0 — `aarch64le`, `x86_64` |
| On-target test | [`test_vsomeip`](../../tests/src/) |

## Install and remove on a target

No build needed: prebuilt packages for `x86_64` and `aarch64`, installed under `/data/oss` (with `/usr/local` linked to `/data/oss/usr/local`; [INSTALL.md](../../INSTALL.md) step 1 prepares a target). Packages: `vsomeip` (programs, shared libraries, run-time data) and `vsomeip-dev` (headers, pkg-config/CMake files, static libraries; needs `vsomeip`).

Each package's page gives the exact file, URL, SHA-256 and dependencies per architecture: [`vsomeip`](https://openqnx.github.io/oss/package/x86_64/vsomeip.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vsomeip.html)) · [`vsomeip-dev`](https://openqnx.github.io/oss/package/x86_64/vsomeip-dev.html) ([aarch64](https://openqnx.github.io/oss/package/aarch64/vsomeip-dev.html)).

| Method | Install | Remove |
|---|---|---|
| **A.** [apk](../../INSTALL.md#2a-install-with-apk) | `apk add vsomeip` (dependencies come along); one version: `apk add vsomeip=<version>` | `apk del vsomeip` (also removes dependencies nothing else needs) |
| **B-1.** [oss-get](../../target/oss-get) | `oss-get update && oss-get install vsomeip` (dependencies first) | `oss-get remove vsomeip` (its files only: the dependencies stay) |
| **B-2.** by hand | download, check, save the file list, unpack (below) | delete the listed files (below) |

`vsomeip-dev` the same way, by its own name (`apk add vsomeip-dev`, `oss-get install vsomeip-dev`, by hand with its own file and list `vsomeip-dev.list`); remove it before `vsomeip`, which it needs.

```sh
# on the QNX target, as root
apk add vsomeip vsomeip-dev
apk add qnx-test-vsomeip && /usr/local/bin/qnx-port-tests/test_vsomeip    # its on-target test, from tests-8.0-<arch>-m-z
apk del vsomeip vsomeip-dev qnx-test-vsomeip
```

By hand (B-2), from the release `packages-8.0-<arch>-m-z`; take `<file>` and `<sha256>` from the package's page (or its release's `INDEX`: `name|version|depends|file|sha256|...`), and install its dependencies first, the same way:

```sh
GH=https://github.com/openqnx/oss/releases/download
mkdir -p /data/oss/dl /data/oss/var/lib/manual && cd /data/oss/dl
curl -fsSLO $GH/packages-8.0-<arch>-m-z/<file>
[ "$(openssl dgst -sha256 <file> | sed 's/.*= *//')" = <sha256> ] && echo OK || echo BAD
gzip -dc <file> | tar -tf - | grep -v '/$' > /data/oss/var/lib/manual/vsomeip.list
gzip -dc <file> | (cd /data/oss && tar -xf -) && rm <file>
# remove: exactly the files it installed
cd /data/oss && while read -r f; do rm -f "$f"; done < var/lib/manual/vsomeip.list
rm var/lib/manual/vsomeip.list
```

With B-1 and B-2 the dependencies stay installed after a remove: remove them too only if no other installed package needs them (`oss-get remove` refuses one that is still needed). Use one method per `/data/oss`: apk and oss-get do not know about files unpacked by hand. Then `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)` sets `PATH` and `LD_LIBRARY_PATH`; [docs/packages.md](../../docs/packages.md) has the details, and your own build can be served from `dists/` ([`scripts/serve-dists`](../../scripts/serve-dists)).

## Build it from upstream

Every step below can be run by hand. `./scripts/qnxbuild vsomeip` performs exactly the same steps (2–5) in one command.

### 1. Build environment

A Linux host with QNX SDP 8.0 installed and licensed — or the container in [`docker/`](../../docs/build-environment.md), which needs only the SDP mounted. Then, from the repository root:

```bash
export QNX_SDP_ROOT=$HOME/qnx800      # your SDP 8.0 installation
./scripts/build-host-tools.sh          # once per machine
source ./env.sh aarch64le              # or x86_64; once per shell
```

`env.sh` sets up the cross compiler (`qcc`/`q++`), the staging tree `stage/<arch>/usr/local` (= `/usr/local` on the target), pkg-config, and the CMake/Meson toolchain files.

**Build the dependencies first** (8 component(s)), or let `qnxbuild` do it:

```bash
./scripts/qnxbuild boost
```

<details><summary>Full build chain (8)</summary>

[`boost`](../boost/) [`bzip2`](../bzip2/) [`gettext`](../gettext/) [`libiconv`](../libiconv/) [`libunistring`](../libunistring/) [`ncurses`](../ncurses/) [`xz`](../xz/) [`zlib`](../zlib/)
</details>

### 2. Get the upstream source

```bash
git clone --depth 1 -b 3.8.0 --recurse-submodules --shallow-submodules \
    https://github.com/COVESA/vsomeip.git src/vsomeip
```

### 3. Apply the QNX patches

5 patch(es), each a separate file in [`patches/`](patches/), applied in order:

```bash
patch -p1 -d src/vsomeip < ports/vsomeip/patches/0001-qnx8-native-accept4.patch
patch -p1 -d src/vsomeip < ports/vsomeip/patches/0003-qnx-sd-without-netlink.patch
patch -p1 -d src/vsomeip < ports/vsomeip/patches/0005-qnx-uds-peer-credentials.patch
patch -p1 -d src/vsomeip < ports/vsomeip/patches/0006-qnx-socket-options.patch
patch -p1 -d src/vsomeip < ports/vsomeip/patches/0007-qnx-cmake-wrap-flags.patch
```

**[`0001-qnx8-native-accept4.patch`](patches/0001-qnx8-native-accept4.patch)** — vsomeip: use QNX 8.0's own accept4().

> The QNX wrapper re-implements accept4() with the _IO_OPENFD message protocol
> for QNX 7.x. QNX 8.0 exports accept4() from libsocket and no longer has
> <sys/sockmsg.h>, which the emulation needs; build it for pre-8.0 targets only.

Touches: `implementation/utility/src/wrappers_qnx.cpp` · Upstream status: Pending

**[`0003-qnx-sd-without-netlink.patch`](patches/0003-qnx-sd-without-netlink.patch)** — Send SD offers on QNX, which has no netlink route notification.

> With service discovery enabled, routing_manager_impl sends offers only when
> is_external_routing_ready(), which needs sd_route_set_. Only the netlink
> connector sets it (route to the SD multicast group reported up), and only
> the netlink callback calls init_pending_services() for offers made before
> routing started. QNX has no netlink: start_ip_routing() sets
> if_state_running_ but neither of the other two, so every offer stays in
> pending_sd_offers_ forever and nothing is ever offered on the network
> ("Didn't receive a multicast SD message" on both sides). The
> wait_route_netlink_notification setting does not help: it only stops the
> route being cleared. On QNX, mark the SD route as set and flush the
> pending offers when IP routing starts. Since 3.8.0 start_ip_routing()
> sets sd_route_set_ on QNX itself; the pending offers are still flushed only
> by the netlink callback, so that part stays.

Touches: `implementation/routing/src/routing_manager_impl.cpp` · Upstream status: Pending

**[`0005-qnx-uds-peer-credentials.patch`](patches/0005-qnx-uds-peer-credentials.patch)** — Get the peer credentials of a UNIX domain socket with getpeereid() on QNX.

> vsomeip 3.8 reads the uid/gid of every local (UDS) peer with
> getsockopt(SOL_SOCKET, SO_PEERCRED) into a struct ucred, built for Linux and
> QNX alike. QNX has neither SO_PEERCRED nor struct ucred (the build stops
> there), but it has getpeereid() in <unistd.h>, which returns the same
> effective uid and gid of the connected peer. Use it on QNX; an error still
> makes the connection fail as before ("Could not getsockopt(SO_PEERCRED)").

Touches: `implementation/endpoints/include/asio_uds_socket.hpp` · Upstream status: Pending

**[`0006-qnx-socket-options.patch`](patches/0006-qnx-socket-options.patch)** — Build the 3.8 socket options on QNX: SO_BINDTODEVICE, IP_PKTINFO, SO_RCVBUFFORCE.

> vsomeip 3.8 moved its socket options into socket_options.hpp and
> asio_tcp_socket.hpp (built for Linux and QNX). They name SO_BINDTODEVICE,
> IP_PKTINFO and SO_RCVBUFFORCE, which QNX 8.0's headers do not define, so the
> build stops there. vsomeip's own qnx_helper.hpp defines the first two for
> QNX (as 3.5 used them) but is not included by these files, and in 3.8.0 it
> lacks the #endif of its #ifdef __QNX__ ("unterminated #ifdef" wherever it is
> included). Close the #ifdef, include qnx_helper.hpp in both files, and use
> SO_RCVBUF for receive_buffer_force on QNX (it is only set under __linux__;
> the template still has to compile).

Touches: `implementation/utility/include/qnx_helper.hpp`, `implementation/endpoints/include/socket_options.hpp`, `implementation/endpoints/include/asio_tcp_socket.hpp` · Upstream status: Pending

**[`0007-qnx-cmake-wrap-flags.patch`](patches/0007-qnx-cmake-wrap-flags.patch)** — Link libvsomeip3 on QNX with only the syscall wrappers QNX has.

> vsomeip 3.8 wraps close, recvfrom, recvmsg, sendto, sendmsg and epoll_* with
> -Wl,-wrap on Linux; the __wrap_* functions for them (wrappers.cpp) are built
> for Linux only, and QNX gets socket/accept/open (wrappers_qnx.cpp). The
> choice is `if(${CMAKE_SYSTEM_NAME} MATCHES "QNX")`: on QNX ${CMAKE_SYSTEM_NAME}
> is "QNX", which if() then reads as CMake's own variable QNX (1), so the test
> is false and QNX got the Linux list -- every program linking libvsomeip3
> failed with "undefined reference to `__wrap_close'" (and recvfrom, sendto,
> sendmsg). Test the variable itself, for the library and the test library.
> (The other `${CMAKE_SYSTEM_NAME} MATCHES "QNX"` tests are left as they were:
> the port has always been built and verified with them false.)

Touches: `CMakeLists.txt`, `test/CMakeLists.txt` · Upstream status: Pending

### 4. Configure, build and install into the staging tree

```bash
cmake -S src/vsomeip -B build/$QNX_ARCH/vsomeip -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=$PWD/toolchain/qnx-$QNX_ARCH.cmake \
      -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 -DBUILD_TESTING=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DENABLE_SIGNAL_HANDLING=1 \
      -DBoost_NO_BOOST_CMAKE=OFF \
      -DBOOST_ROOT="$STAGE$PREFIX" \
      -DBoost_INCLUDE_DIR="$STAGE$PREFIX/include" \
      -DBoost_LIBRARY_DIR="$STAGE$PREFIX/lib" \
      -DDISABLE_DLT=1
cmake --build build/$QNX_ARCH/vsomeip -j$(nproc)
DESTDIR=$STAGE cmake --install build/$QNX_ARCH/vsomeip
```

Or all of steps 2–4 at once: `./scripts/qnxbuild -a $QNX_ARCH vsomeip`.

### 5. What it installs

40 files under `/usr/local`: 3 shared-library entries, 26 headers.

```
/usr/local/lib/libvsomeip3.so
/usr/local/lib/libvsomeip3.so.3
/usr/local/lib/libvsomeip3.so.3.8.0
```

The complete list is `stage/<arch>/.manifest/vsomeip` after a build.

### 6. Package it for the target

```bash
./scripts/qnxpkg -a $QNX_ARCH --key keys/private/<key>.rsa vsomeip
ls dists/8.0/*/ | grep '^vsomeip-'
```

This writes `vsomeip-<ver>-r0.tar.gz` (run time) and `vsomeip-dev-<ver>-r0.tar.gz` (headers, pkg-config, CMake files, static libraries), the matching `.apk` files, and updates the repository index. The run-time dependencies recorded in the package are computed from the binaries.

## Run it on a target

Serve your build from `dists/` and install it with apk:

```bash
./scripts/serve-dists &                                  # on the build host: http://<host>:8000
./scripts/target-bootstrap <target> -r http://<host>:8000  # once per target
```
```sh
# on the target
. /data/oss/etc/oss-env.sh
apk update && apk add vsomeip
```

or copy the tarball and unpack it yourself:

```bash
scp dists/8.0/aarch64/vsomeip-*-r0.tar.gz root@<target>:/data/
ssh root@<target> 'mkdir -p /data/oss && cd /data/oss && tar xzf /data/vsomeip-*-r0.tar.gz'
```

QNX has no `ld.so.conf`; make the libraries findable with `. /data/oss/etc/oss-env.sh` (apk) or `eval $(oss-get env)`.

### Test

[`test_vsomeip`](../../tests/src/) links vsomeip the way an application would and checks its core API on the target. It is a package of its own, `qnx-test-vsomeip`, in the tests repository (`test_suites/8.0/<arch>/`, `scripts/qnxpkg --tests`; on GitHub the release `tests-8.0-<arch>-m-z`), which apk-setup adds next to the package repository:

```sh
# on the target
apk add qnx-test-vsomeip
/usr/local/bin/qnx-port-tests/test_vsomeip
```

```
== vsomeip ==
  ok   ...
PASS vsomeip
```

`apk add qnx-port-tests` adds `run-on-target.sh`, which runs every installed test. Without apk: `./tests/build-tests.sh $QNX_ARCH`, copy `stage/$QNX_ARCH/usr/local/bin/qnx-port-tests/test_vsomeip` over and run it with `LD_LIBRARY_PATH=/usr/local/lib`; `./tests/make-bundle.sh && ./tests/deploy-and-run.sh <arch> <target>` runs every test at once.

## How it was verified

1. **Built** from the original source with the patches above for `aarch64le`, `x86_64`, by `scripts/qnxbuild` from a clean build directory.
2. **Staging audits** (`scripts/check-all`) over what it installs: every undefined symbol resolvable, no library taken from the SDP where this tree builds one, development links present, no duplicate files, no host binaries or build paths, no private copies of libc, every DT_NEEDED a plain soname.
3. **On the target**: [`test_vsomeip`](../../tests/src/) -- a program linked against vsomeip the way an application would be -- run on QNX 8.0, each check printing `ok`/`FAIL`:

   - fork consumer process
   - runtime::get
   - create_payload
   - payload length
   - payload content
   - create_request
   - service id readback
   - method id readback
   - payload attached to message
   - create_application
   - application name readback
   - init as routing manager
   - SD OfferService seen on the multicast group
   - consumer process subscribed over local routing
   - consumer received all events in order and all method replies
   - provider served every method call

Results recorded in [`tests/results/`](../../tests/results/) (`scripts/record-results`, `scripts/verify-on-target`):

| Arch | Target | Installed from | Version | Result | Date | Log |
|---|---|---|---|---|---|---|
| aarch64le | QEMU aarch64 virt (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vsomeip 3.5.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/vsomeip-3.5.6-r0.log) |
| aarch64le | QEMU aarch64le, vanilla mkqnximage image | apk:local-dists | vsomeip 3.5.6-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/aarch64le/vsomeip-3.5.6-r1.log) |
| x86_64 | QEMU x86_64 (mkqnximage image + SDP runtime libraries) | bundle | – | ✅ PASS | 2026-10-30 | – |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vsomeip 3.5.6-r0 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/vsomeip-3.5.6-r0.log) |
| x86_64 | QEMU x86_64, vanilla mkqnximage image | apk:local-dists | vsomeip 3.5.6-r1 | ✅ PASS | 2026-11-01 | [log](../../tests/logs/x86_64/vsomeip-3.5.6-r1.log) |

The targets are stock QNX SDP 8.0 QEMU images; [docs/qemu-test-vms.md](../../docs/qemu-test-vms.md) shows how to build the same ones and run the same commands yourself.

## Versions

This recipe builds **3.8.0**. Kept from earlier: `3.5.6` ([recipe and patches](versions/3.5.6/)). Every version stays available: packages in `dists/8.0/<arch>/` (`apk add vsomeip=<version>`), sources in `ported/vsomeip/<version>/`, results per version in `tests/results/`. Newer upstream releases: `scripts/check-updates vsomeip`; port one with `scripts/port-update vsomeip`.

<details><summary>recipe.sh</summary>

```bash
VERSION=3.8.0; GIT=https://github.com/COVESA/vsomeip.git; REF=3.8.0
DEPS="boost"
CMAKE_ARGS_EXTRA=( -DBUILD_SHARED_LIBS=ON -DENABLE_SIGNAL_HANDLING=1
                   -DBoost_NO_BOOST_CMAKE=OFF
                   -DBOOST_ROOT="$STAGE$PREFIX"
                   -DBoost_INCLUDE_DIR="$STAGE$PREFIX/include"
                   -DBoost_LIBRARY_DIR="$STAGE$PREFIX/lib"
                   -DDISABLE_DLT=1 )
```
</details>

---
<sub>Generated by <code>scripts/gen-port-docs</code> from <code>recipe.sh</code>, <code>patches/</code> and the build manifest. Edit those, not this file.</sub>
