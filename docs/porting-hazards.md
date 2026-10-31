# QNX porting hazards

Everything that bit us while porting 220 components to QNX SDP 8.0, and how each
was handled. The project overview lives in the [root README](../README.md).

Cross-builds automotive open source for **aarch64le** and **x86_64** from upstream
sources. Nothing prebuilt from the SDP or from `repo.oss.qnx.com` is reused.

    SDP:      $QNX_SDP_ROOT   (QNX 8.0, qcc / gcc 12.2.0)
    Prefix:   /usr/local            (on target)
    Staging:  stage/<arch>/usr/local

## Documents
    docs/qnx-build-and-deploy-guide.html   follow-along: build from scratch, then deploy to a Pi 5
    docs/qnx-port-recipes.html             per-package source URL, flags and patches (generated)
    docs/qnx-ports-summary.html            the ported set at a glance
    docs/qnx-porting-guide.html            QNX porting patterns

## Layout
    env.sh              source it to enter the build environment
    toolchain/          CMake toolchain files + Meson cross files, per arch
    ports/<pkg>/      recipe.sh (+ patches/*.patch applied on first fetch)
    scripts/qnxbuild    the driver
    src/  build/  stage/  logs/  dist/

## Usage
```bash
cd ~/oss
./scripts/qnxbuild -l                 # status of every recipe
./scripts/qnxbuild vsomeip            # build for both arches
./scripts/qnxbuild -a aarch64le -f X  # one arch, force rebuild
```
A recipe declares `GIT`/`REF` or `TARBALL`, `BUILDSYS` (cmake / meson / autotools /
custom), `DEPS`, and extra flags; `do_configure` / `do_build` / `do_install` can be
overridden for anything that doesn't fit. Deps are built recursively, and every
package installs into the per-arch staging tree that later packages discover.

---
## QNX porting hazards (learned here, handled centrally in `toolchain/` + `env.sh`)

**1. `-std=c++17` hides POSIX.** QNX headers gate declarations behind
`__EXT_POSIX1*` / `__EXT_QNX`, which strict-ANSI mode switches off. `-std=c++17`
makes `::stat`, `::fstat`, `::getpid`, `::isatty` disappear; `-std=gnu++17` does
not. Fixed globally with `add_compile_definitions(_QNX_SOURCE)` plus
`CMAKE_C/CXX_EXTENSIONS ON`.

**2. libc is split.** Sockets are in `libsocket`; `__isinf`/`__isnan` and the rest
of math are in `libm`. Neither is implicit — both are in
`CMAKE_*_STANDARD_LIBRARIES_INIT`.

**3. `-lrt -ldl -lpthread -lcrypt -lutil -lresolv -lanl` do not exist** — QNX folds
them all into libc, but portable build systems emit them unconditionally.
`env.sh:qnx_make_compat_stubs()` drops empty archives of each into the staging lib
dir so the flags resolve to no-ops. Cheaper and more robust than patching every
package.

**4. CMake identifies qcc as `QCC`, not `GNU`.** Any project branching on
`CMAKE_C_COMPILER_ID MATCHES "GNU"` silently takes the "unknown compiler" path.
Seen in cyclonedds/idlpp, where it left `HOST_COMPILER` empty and turned every
`#if HOST_COMPILER == ...` into a syntax error.

**5. `link_directories()` in a toolchain file does not reach project scope.** The
staging `-L`, and `-Wl,-rpath-link` for transitive `NEEDED` libs, have to go in
`CMAKE_{EXE,SHARED,MODULE}_LINKER_FLAGS_INIT`. Without `-rpath-link`, a shared
library links fine but every executable that consumes it fails on its
dependencies-of-dependencies.

**6. No xlocale.** No `locale_t`, `newlocale`, `uselocale`, or any `*_l` function.
QNX has only the "C" locale, so mapping the `_l` forms onto the plain ones is
semantically exact (see `ports/cyclonedds/patches/0001`).

**7. `SA_RESTART` is unimplemented** — `<signal.h>` keeps it commented out as "not
supported yet". Boost.Asio's `signal_set_base` uses it unconditionally.

**8. `__QNX__` is predefined as `700` / `710` / `800`.** Use it to separate SDP 8.0
from older QNX. vsomeip ships an `accept4()` emulation for QNX 7.x built on
`<sys/sockmsg.h>`; SDP 8.0 exports a real `accept4()` from `libsocket.so.4` and has
deleted that header.

**9. b2's `qcc` toolset is broken in Boost 1.86.** `toolset=qcc` segfaults b2 5.2.1
during configuration, with or without a user-config. Declaring the QNX cross-GCC
driver as a *named gcc toolset* (`using gcc : qnx : .../aarch64-unknown-nto-qnx8.0.0-g++`)
works. Also, b2 exempts `target-os=qnx` from its "`-pthread` plus `-lrt`" rule but
not `target-os=qnxnto` — and Boost.Log's Jamfile requires `qnxnto`.

**10. Legacy net media types are absent.** `IFM_ETHER` and `IFM_IEEE80211` exist;
`IFM_TOKEN` and `IFM_FDDI` do not.

**11. QNX's C++ runtime is libc++, which hard-errors below C++11.** CMake probes
that clear `CMAKE_REQUIRED_FLAGS` then compile at the compiler default and fail
for reasons unrelated to what they test — Eigen's math-library check reports
"Can't link to the standard math library". Fixed by putting `-std=gnu++17` in
`CMAKE_CXX_FLAGS_INIT`, which every probe inherits.

**12. libc++ vs libstdc++ differ in `system_clock::duration`** — microseconds on
libc++, nanoseconds on libstdc++. Code that builds a `time_point` straight from
`nanoseconds` compiles on Linux and fails on QNX (narrowing). Needs an explicit
`duration_cast` (see `ports/lely-core/patches/0001`).

**13. No `<endian.h>`.** The BSD spellings `BYTE_ORDER` / `LITTLE_ENDIAN` live in
`<sys/param.h>`; the glibc `__BYTE_ORDER` / `__LITTLE_ENDIAN` names do not exist.
`htobe32` / `be32toh` and friends are in `<net/netbyte.h>`.

**14. `uid_t` and `gid_t` are signed `int`** (`__UID_T` is `_INT32`), so the
idiomatic `0xffffffff` sentinel is a narrowing conversion inside brace-init.

**15. There is no static `libsocket.a`** — only the shared object. Any target
linked `-static` that touches sockets cannot link; disable such targets (this is
what breaks foonathan_memory's `nodesize_dbg`).

**16. Feature detection keyed on `_POSIX_C_SOURCE` fails.** QNX gates its POSIX
surface on `__EXT_POSIX*` instead, so portability shims conclude the platform is
missing `<unistd.h>`, `struct timespec`, `struct itimerspec`, `getopt` … and then
supply their own, which collide with libc at compile or link time. lely-core hits
this four separate ways.

**17. Absent Linux-only facilities seen so far:** `inotify` (Fast-DDS file
watch), `cpu_set_t` / `pthread_setaffinity_np` (use `ThreadCtl(_NTO_TCTL_RUNMASK)`
— the affinity bitmask maps across directly), `SCHED_BATCH` / `SCHED_IDLE`,
`<sys/sysinfo.h>` / `get_nprocs_conf()` (use `sysconf(_SC_NPROCESSORS_CONF)`),
`<sys/syslog.h>` (QNX has the standard `<syslog.h>`), SocketCAN, and
`TCPCTL_*` — QNX 8.0 has the standard `TCP_KEEPIDLE` / `TCP_KEEPINTVL` /
`TCP_KEEPCNT` instead.

**18. qcc misparses `-MMD -MF <file> -c -o <obj> <src>`** as multiple input files
("Can't specify -P, -C, -E, -c or -S with -o and have multiple files"). OpenSSL's
makefiles are built that way. Driving the cross-GCC
(`aarch64-unknown-nto-qnx8.0.0-gcc`) instead of qcc avoids it — the same escape
hatch that b2 needed.

**19. CMake 4 dropped `cmake_minimum_required(VERSION < 3.5)`.** Passing
`-DCMAKE_POLICY_VERSION_MINIMUM=3.5` covers the top-level configure but not
nested `cmake -P` script runs (OpenCV's pkgconfig generator), so `env.sh` exports
it into the environment as well.

**20. `check_*_source_runs()` can never succeed when cross-compiling.** CMake
marks the result FAILED_TO_RUN, and packages that gate real functionality on it
break far from the probe: PCL's aligned-allocation check fails this way and
`pcl_macros.h` then `#error`s with "aligned_malloc not supported on your
platform". Seed the `<VAR>_EXITCODE` cache entry after verifying the feature
against the SDP by hand.

**21. qcc's option forwarding is the single most repeated obstacle.** Four
distinct failures in this workspace, all the same shape — qcc accepts a flag and
then does not pass it on:
| symptom | what qcc dropped | fix used |
|---|---|---|
| "Can't specify -P, -C, -E, -c or -S with -o and have multiple files" | `-MF <file>` read as a second input | drive the cross-GCC (OpenSSL) |
| b2 segfaults configuring `toolset=qcc` | — | declare the cross-GCC as a named `gcc` toolset (Boost) |
| "selected processor does not support `fmla v20.8h…`" | `-march=…+fp16` not reaching `as` | `CMAKE_ASM_COMPILER` = cross-GCC |
| "cc: unknown option: '--version-script=…'" / "'-z'" | `-Xlinker`, bare `-z` | rewrite as `-Wl,…` (ONNX Runtime) |

The pattern: when a build system hands the compiler driver anything beyond plain
compile flags, prefer `aarch64-unknown-nto-qnx8.0.0-gcc` over `qcc`. Both drive
the same GCC 12.2.0 and produce identical QNX binaries.

**22. A global staging `-I` can shadow a package's vendored dependencies.**
Needed so that `#include <lz4.h>` resolves (FLANN), but it also lets the
workspace's protobuf/Eigen outrank the versions a package pins for itself.
ONNX Runtime needed both `FETCHCONTENT_TRY_FIND_PACKAGE_MODE=NEVER` and
`CMAKE_DISABLE_FIND_PACKAGE_Eigen3=ON` before it used its own.

**23. cpuinfo has no QNX backend.** It compiles to an empty library with
`CPUINFO_SUPPORTED_PLATFORM=0`, while consumers that only check the CPU
architecture still call into it and fail at link time on `cpuinfo_initialize`.

**24. x86_64 `memchr` takes its length as signed.** QNX 8's libc compares the
length with a signed `cmovle`, so `memchr(s, c, SIZE_MAX)` -- the usual way to
say "no limit", and what `strndup(s, SIZE_MAX)`-style helpers pass -- indexes
its alignment jump table at -1 and jumps to address 0. ROS 2's
`rcutils_strdup()` does exactly this from librcl's load-time constructor:
every node died with SIGSEGV before `main()`. Bound the length by
`PTRDIFF_MAX` (`ports/ros2/repo-patches/rcutils-qnx-memchr-length.patch`).

**25. `openat()` needs a directory fd opened with `O_DIRECTORY`.** A directory
opened plain `O_RDONLY` works with `read`/`fstat`, but every `openat()` under
it fails with `ENOTDIR`. apk-tools opened its database directory that way, so
any second `apk` command on a root failed "Unable to read database: Not a
directory" (`ports/apk-tools/patches/0003-*`).

**26. `--as-needed` can leave a library exporting its own libc.** When a
shared library itself calls nothing in libc, `--as-needed` drops `libc.so`,
and what crt/libgcc_eh still need comes from the static `libcS.a` -- exported:
`memchr`, `malloc`, `sigaction` and some 200 more, to which every library
loaded later then binds. Meson uses `--as-needed` by default; 32 libraries
here had it. The toolchain files keep libc needed with
`-Wl,--push-state,--no-as-needed -lc -Wl,--pop-state`;
`scripts/check-embedded-libc` audits for it.

**27. Anonymous memory is committed when it is mapped.** QNX has no
overcommit: a buffer `mmap`ed and never touched is still RAM. OpenBLAS sized
its thread pool at 64 on QNX (the OS was missing from its CPU-count clamp) and
reserved 128 MB per thread at load: any program linking it consumed a 2 GB
target's memory before `main()` (`ports/openblas/patches/0003-*`). Look for
large up-front reservations that are free on Linux.

**28. Shared objects are initialised in reverse load order.** QNX's loader runs
constructors in the reverse of the order it loaded objects, not in dependency
order: a library loaded before its user is initialised after it (with libc
first in an executable's `DT_NEEDED`, libc's own initialisers run after every
other library's). Constructors that rely on another library's constructors
having run cannot assume it.

**29. A stock target has no CA certificates.** The SDP's OpenSSL looks for
`/usr/ssl/cert.pem`, which neither the SDP nor an image ships: `curl` and
anything else using it fail to verify any `https://` server (curl error 60).
The `ca-certificates` component installs a bundle at this tree's OpenSSL
default, `/usr/local/ssl/cert.pem`; the first download onto a fresh board
needs one copied over (`scripts/target-bootstrap`).

**30. CMake `VERSION` from undefined variables.** unibilium's
`VERSION "${VERSION_MAJOR}.${VERSION_MINOR}"` names variables nothing sets,
giving the soname `libunibilium.so..`; and a module linked by path that has no
soname (LPeg's `lpeg.so`) puts the build host's path in the consumer's
`DT_NEEDED`. Both fail at load on the target; `scripts/check-needed` audits
every staged ELF file for them.

---
## Port verification tests

`tests/` holds one QNX binary per ported package -- **165 of them**. Each
exercises the library's real API, not
just its headers: zlib does a compress/uncompress round trip, OpenSSL checks
SHA-256("abc") against the FIPS 180-4 vector, Cyclone DDS creates a participant,
publisher and subscriber, Ceres solves a least-squares problem, PCL builds a
cloud and runs a voxel filter and a kd-tree search, ONNX Runtime builds an Env
and a tensor.

```bash
./tests/build-tests.sh               # both arches
./tests/make-bundle.sh               # minimal on-target tarballs, both arches
./tests/deploy-and-run.sh aarch64le <target-ip>    # ship it and run it there
./tests/deploy-and-run-chunked.sh x86_64 <host> 35   # same, for a target too
                                                     # small to hold the bundle
./scripts/check-undefined            # unresolved symbols in staged libraries
./scripts/check-devlinks             # missing lib<name>.so symlink, or an empty .so
./scripts/check-sdp-leak             # binaries that load an SDP prebuilt library
./scripts/check-dupes                # one library staged under two sonames
./scripts/check-artifacts            # a staged ELF that is not QNX/<arch>, a
                                     # dangling symlink, or a build path baked
                                     # into an installed .pc/.la
./scripts/check-test-coverage <arch> # a package no test links, includes or runs
```

Binaries install to `stage/<arch>/usr/local/bin/qnx-port-tests/` with a
`tests.list` manifest and `run-on-target.sh`. Copy `stage/<arch>/usr/local` to a
QNX target as a `/usr/local` overlay and run:

```sh
export LD_LIBRARY_PATH=/usr/local/lib
/usr/local/bin/qnx-port-tests/run-on-target.sh
```

Cross-built binaries cannot execute on the Linux build host, so building them
proves headers, libraries and link lines work for a downstream consumer;
running them on a QNX target is what proves the ports behave.

### Result on hardware
**165 / 165 pass on both architectures**, against a
staging tree rebuilt from scratch -- every source re-cloned, every patch
re-applied from `ports/`:

| arch | target | result |
|---|---|---|
| aarch64le | `QNX qnxpi 8.0.0 RaspberryPi5 aarch64le` (hardware) | **165 / 165** |
| x86_64 | `QNX 8.0.0 x86pc x86_64` (QEMU, 146 MB image) | **165 / 165**, over NFS |

Both rows are the full suite, taken after the from-scratch rebuild.

The x86_64 image is a default `mkqnximage` build: a 146 MB disk, no `libasound`,
and none of QNX's split-libc shared libraries, so nothing this workspace
produces even loads on it unaided. `tests/run-over-nfs.sh` mounts the build
host's already-exported home directory read-only with `fs-nfs3` (which needs
root on the target) and runs the suite in place, carrying the split-libc
runtime from the SDP. That removes the size limit entirely -- one test's
library closure alone is 181 MB. `tests/deploy-and-run-chunked.sh` remains for
a target with no NFS route: it ships one batch at a time, skipping tests whose
closure exceeds the budget.

`tests/make-bundle.sh` packages the binaries with only the shared libraries they
pull in through DT_NEEDED, plus the runtime data some of them read from disk
(the XKB rules, fontconfig's config, the terminfo database, and the GStreamer
plugin modules and their scanner helper, which are dlopened and so never appear
in DT_NEEDED) -- about 140 MB rather than the 1.5 GB staging tree. Then
`run-on-target.sh` drives the suite:

```sh
tar xzf qnx-port-tests-<arch>.tar.gz && cd qnx-port-tests-<arch>
export LD_LIBRARY_PATH=$PWD/lib
sh bin/run-on-target.sh
```

### Real defects the tests found
Each of these produced a library that built and linked cleanly. Only running the
code on a real target exposed them.

- **libffi handed out closures in non-executable memory.** `ffi_closure_alloc()`
  returns a writable pointer and an executable one; libffi only uses its
  `mmap(PROT_EXEC)` allocator when `FFI_MMAP_EXEC_WRIT` is defined, and that is
  gated on `__linux__` or Windows. Everywhere else it falls back to plain
  `malloc`, so on QNX calling through a closure jumped into the malloc arena:
  `gdb` on the target put `rip` at `__main_arena+2240` in libc. Every libffi
  callback on QNX segfaulted -- which is the mechanism behind GObject
  introspection, Python `ctypes`, and any language binding built on libffi.
  Fixed by enabling the executable-mapping allocator for QNX, with the
  temporary-file fallback list (`$TMPDIR`, `/tmp`, `/var/tmp`) rather than
  Linux's `<mntent.h>` filesystem scan.
- **Orc registered no compile target at all on aarch64.** Its ARM detection
  reads `/proc/cpuinfo`; QNX has no procfs, so the probe returned no flags, the
  NEON backend was marked non-executable, and `orc_target_get_default()`
  returned nothing -- every Orc program (so every SIMD fast path in GStreamer's
  audio/video converters) fell back to the interpreter. Advanced SIMD is
  architecturally mandatory on ARMv8-A, so the fix is the one Windows-ARM64 and
  macOS-arm64 already use: report it unconditionally. The x86_64 target was
  unaffected, which is why only the Raspberry Pi run caught it.
- **readline installed no `libreadline.so`.** `support/shlib-install` creates the
  development symlink only for hosts it recognises, and `nto-qnx` is not one, so
  staging held `libreadline.so.8` alone. `-lreadline` is not an error in that
  situation: the linker keeps searching and finds the SDP's *prebuilt*
  libreadline -- silently linking against exactly the binary this workspace is
  required never to use. `scripts/check-devlinks` now audits every staged
  library for this, and flags loudly when the SDP has a same-named library ready
  to take over.
- **`libyuv.so` had unresolved `jpeg_*` symbols.** `-DHAVE_JPEG` compiles the
  MJPEG decoder into the libraries, but upstream links libjpeg only into the
  `yuvconvert` tool, so the shared library shipped with eight holes and no
  `DT_NEEDED` on libjpeg.
- **`libSvtAv1Enc.so` had an unresolved `cpuinfo_isa`.** The bundled cpuinfo
  excludes QNX from its supported-OS list, so the CPUID-based `src/x86/*.c` were
  never compiled -- while SVT-AV1's own runtime dispatch still referenced the
  ISA table. The BSD initialiser is pure CPUID and works verbatim on QNX.

- **`libCommonAPI-SomeIP.so` had an unresolved `eventfd`.** QNX puts `eventfd`
  in its own `libeventfd`, not libc — the same libc-splitting pattern as
  libsocket and libm (hazard #2). `-Wl,--allow-shlib-undefined`, which shared
  libraries need during the build, let the hole through silently.
- **`libgrpc.so` had an unresolved `grpc_core::MakeDirectoryReader`.** gRPC's
  POSIX `directory_reader.cc` is gated on a hardcoded platform list that did not
  name QNX, so the function was never compiled. Its body uses only
  `opendir`/`readdir`/`d_name`, all of which QNX supports.

- **CommonAPI aborted at process exit** with `mutex lock failed: Invalid
  argument`. Every API check passed first, so only running it exposed this. A
  backtrace on target put it precisely: `CommonAPI::SomeIP::FactoryDeinit()` ->
  `Runtime::unregisterFactory()` -> `std::mutex::lock` returning EINVAL.
  `factoriesMutex_` is an *instance member* of `Runtime`, and libCommonAPI's own
  ELF finaliser had already destroyed that object -- a use-after-free between two
  DSOs whose finaliser order neither controls. glibc's `pthread_mutex_lock`
  tolerates the stale mutex; QNX returns EINVAL, libc++ throws, and the process
  aborts. Upstream already carries a TODO about a crash in this exact function.
  Fixed by not destroying the singleton at finalisation
  (`ports/capicxx-core-runtime/patches/0001`): the OS reclaims it at exit
  anyway, and leaving it alive keeps late `unregisterFactory()` calls valid.
  Isolation, on target: CommonAPI core alone exits cleanly, vsomeip alone exits
  cleanly, the two together abort.

Three audits guard the staging tree, and both architectures are clean on all of
them:

| script | what it catches |
|---|---|
| `check-undefined` | strong undefined symbols that neither a library's dependencies nor QNX itself can satisfy — the `libyuv`/`libSvtAv1Enc` class |
| `check-devlinks` | a library staged without its `lib<name>.so` development symlink, which silently redirects `-l<name>` to the SDP's prebuilt copy |
| `check-sdp-leak` | a staged binary whose `DT_NEEDED` names a soname we do not provide but *do* build — i.e. it loads the SDP's prebuilt library on the target |

The brief for this workspace is that nothing prebuilt from the SDP is reused.
The first two checks are about correctness; the third is what actually enforces
that rule, because the failure it catches is completely silent.

---
## Native helper tools

`scripts/build-host-tools.sh` builds the tools a cross build must *run* on the
build machine, into `hosttools/` (on PATH via `env.sh`):

- **gettext** — GLib, GStreamer and most of GNOME invoke `xgettext`/`msgfmt`
  during the build; the copies in `stage/` are QNX binaries and cannot run here.
- **nasm** — the x86_64 assembly in dav1d, openh264, libvpx and FFmpeg is NASM
  syntax. Without it those builds fail outright or silently fall back to C.
  (aarch64 assembly is GNU `as` syntax and needs nothing extra.)
- **wayland-scanner** — turns Wayland's XML protocol definitions into C.

`toolchain/meson-native.ini` points meson at these for build-machine
dependencies, via `hosttools/bin/pkg-config-native` — a wrapper that clears
`PKG_CONFIG_SYSROOT_DIR`, which otherwise rewrites native absolute paths into
the QNX staging tree.

Neither is installed system-wide, so no root is required.

## Cross-build hazards found while porting the GNOME/multimedia stack

**24. meson's link probes need `-shared-libgcc` on QNX.** meson wraps every
check in `-Wl,--start-group ... --end-group`. QNX's `libm.so` references
`__multc3`, a *hidden* symbol in the static `libgcc.a`, and ld rejects that with
`final link failed: bad value`. The result is that **every** meson probe fails,
including `find_library('m')` -- with no hint as to why. Taking libgcc from the
DSO fixes it, and it is now in both meson cross files.

**25. pkg-config's sysroot is the staging tree, not the SDP.** The only `.pc`
files in play are the ones this workspace installs, and they carry the on-target
prefix `/usr/local`. Pointing `sys_root` at the SDP turns their
`-I/usr/local/include` into a path that does not exist -- GLib compiles with
`-Werror=missing-include-dirs` and stops. QNX SDP 8.0 ships no pkgconfig
directory of its own.

**26. The cross-GCC has a foreign SDP path compiled in.** `-print-search-dirs`
reports `/opt/qnx800_ga/target/qnx/...`, not this installation, so tools that
enumerate the compiler's library directories (meson among them) look in the wrong
place. `--sysroot` redirects the `=`-prefixed entries.

**27. `cc.has_function()` reports false positives.** meson accepts
`sysctlbyname` on QNX, but libc exports no such symbol -- only the
kernel-internal `kernel_sysctlbyname` -- and linking fails. Anything gated on
such a probe needs checking by hand.

**28. autoconf needs `CPPFLAGS` as well as `CFLAGS`.** Header probes run with
`CPPFLAGS` only. Setting just `CFLAGS` lets configure record the SDP's version of
a library while compilation sees ours: libpng detected zlib 1.2.13 from the SDP
and then compiled against our 1.3.1, tripping its own `ZLIB_VERNUM` guard.

**29. Staging must precede the SDP on the include path.** The SDP ships its own
copies of several of these libraries. Since the brief is to build everything from
source, ours have to win.

**30. `--cross-prefix` makes FFmpeg look for `<prefix>pkg-config`.** Not finding
one, its configure silently substitutes `false`, and every dependency probe then
fails with "not found using pkg-config". Pass `--pkg-config=pkg-config`.

**31. `-Wl,-rpath-link` is needed wherever a dependency has dependencies.**
libvorbis.so needs libogg.so.0; without it the linker cannot resolve the second
hop and reports undefined references that look like a broken library.

**32. Upstream QNX branches often target QNX 6.x/7.x.** x264's configure has a
`*qnx*` case that puts `extras/` on the include path for its getopt replacement
— but `extras/inttypes.h` opens with `#error "Use this header only with
Microsoft Visual C++ compilers!"`, and it shadows QNX 8.0's perfectly good
`<inttypes.h>`. The same pattern appeared in vsomeip (an `accept4` emulation for
QNX 7.x) and libuv (QNX 7.x syspage field names). Always check what an existing
QNX branch was written against.

**33. CMake 4 rejects `cmake_policy(SET ... OLD)`.** x265 sets CMP0025 and
CMP0054 to OLD; both must move to NEW, which is also the correct behaviour.

**34. `--disable-asm` is not always a maintained path.** x264 with assembly
disabled drops struct members its C code still references
(`x264_pixel_function_t has no member named 'ads'`). On aarch64 the assembly is
GNU `as` syntax and builds fine, so only x86 ever needs the switch — and then
nasm is the better answer.

**35. meson's cross file needs `-Wl,-rpath-link` too.** Same root cause as #31,
but far more damaging here: a probe that links against a staged library with its
own dependencies (libfontconfig -> libexpat, libfreetype -> libbrotlidec) fails
for reasons unrelated to what is being tested, and meson records the *feature* as
missing. cairo concluded QNX has no `ctime_r` and defined a conflicting static
fallback. Any meson misdetection is worth re-checking against this.

**36. `-ansi` cannot be used against QNX headers.** It means C90, where `inline`
is not a keyword, and QNX's own `<stdlib.h>` declares `atexit` as
`static inline` -- so the build fails *inside the system headers*
("expected ';' before 'int'"). FriBidi adds `-ansi` for GCC unconditionally.

**37. ICU's QNX makefile fragment is from the QNX 6 era.**
`config/mh-qnx` sets `SHAREDLIBCFLAGS = -shared`, but that variable holds
*compile* flags; the objects come out non-PIC and the aarch64 link fails with
"relocation R_AARCH64_ADR_PREL_PG_HI21 ... recompile with -fPIC". Every other
modern `mh-*` uses `-fPIC`. ICU also needs a native build passed via
`--with-cross-build`, since its data tooling runs on the build machine.

**38. QNX puts each Linux-style event fd in its own shim library.** Beyond
libsocket and libm: `libeventfd`, `libepoll`, `libsignalfd`, `libtimerfd`.
Portable code calls `epoll_create`/`signalfd`/`timerfd_create` without ever
naming a library, so all four belong on every link line (Wayland needs three of
them). They are in the toolchain files.

**39. `open_memstream` does not exist on QNX** — and neither does
`fopencookie` or `funopen` to build one from. Wayland uses it in its
`WAYLAND_DEBUG` logging path; the port backs it with `tmpfile()` and reads the
buffer back at close (`ports/wayland/patches/0001`).

**40. Peer credentials come from `getpeereid()`, not `SO_PEERCRED`.** QNX
supplies uid and gid but no pid. Wayland's FreeBSD branch already reports
`*pid = 0` where the platform cannot provide one, so the QNX branch follows that
precedent.

**41. Wayland rejects a world-writable `XDG_RUNTIME_DIR`.** It must be a 0700
directory owned by the caller, so `/tmp` itself will not do — the on-target test
runner creates a private one.

**42. `<linux/input.h>` button codes are Wayland protocol constants.** GTK3's
GDK Wayland backend uses `BTN_LEFT`/`BTN_RIGHT`/`BTN_MIDDLE`/`BTN_STYLUS*`, and
QNX has neither `<linux/input.h>` nor `<dev/evdev/input.h>`. These are not kernel
values in this context: the `wl_pointer` protocol fixes them to the Linux evdev
numbering on every platform, so defining them directly is correct — GTK already
does exactly that for `BTN_STYLUS3`.

**43. GTK3 widgets require a display.** `gtk_init()` must succeed before any
widget call; using widgets after `gtk_init_check()` fails is undefined and
segfaults. That is not a porting defect, and a test must not pretend otherwise.

**44. GObject types register lazily.** `g_type_from_name("GtkWidget")` returns 0
until something has called `gtk_widget_get_type()`. A test that checks types by
name before touching the library proves nothing — worse, `g_type_is_a(0, 0)` is
trivially true and hides the mistake.

**45. QNX has no abstract UNIX socket namespace.** The Linux `\0`-prefixed
addresses do not exist. GTK's Broadway backend binds and connects with
`G_UNIX_SOCKET_ADDRESS_ABSTRACT` unconditionally; `broadwayd` already falls back
to a filesystem socket when the bind fails, but the client did not, so the two
ends disagreed and no display could be opened. Both now pick the address type
from `g_unix_socket_address_abstract_names_supported()`
(`ports/gtk3/patches/0001`). Worth checking in any IPC code that assumes the
abstract namespace.

**46. libvpx installs into `DIST_DIR`, not `DESTDIR`.** Passing `DESTDIR` alone
runs to completion and installs nothing, so the failure only shows up later when
a consumer cannot find `vpx/vpx_encoder.h`. Its aarch64 runtime CPU detection
also reads Linux hwcaps through `getauxval` and has no QNX path — the source
says as much and points at `--disable-runtime-cpu-detect`, which leaves the
baseline NEON every aarch64 part has.

**47. Executable memory is opt-in, and most JITs gate it on `__linux__`.**
Anything that generates code at run time — libffi closures, Orc, and by
extension every language binding built on them — needs pages mapped
`PROT_EXEC`. libffi's `FFI_MMAP_EXEC_WRIT` block names Linux, Cygwin, Win32 and
OS/2 and nothing else, so on QNX it silently fell back to `malloc` and every
closure call jumped into the heap. QNX *does* allow an anonymous
`PROT_READ|PROT_WRITE|PROT_EXEC` mapping; the fix is to say so. When a JIT
"works but is slow" or faults on the first generated call, this is the first
place to look.

**48. Run-time CPU detection reads `/proc/cpuinfo` or the ELF auxv, and QNX has
neither.** Orc's ARM path returns no flags, marks its NEON backend
non-executable, and ends up with *no* compile target — every program
interpreted, no error printed. libvpx and libaom hit the same wall (hazards #46
and `CONFIG_RUNTIME_CPU_DETECT=0`). On aarch64 the honest answer is that
Advanced SIMD is architecturally mandatory on ARMv8-A, which is exactly what
these projects already assume for Windows-ARM64 and macOS-arm64.

**49. A missing `lib<name>.so` symlink is not a link error — it is a silent
downgrade.** If staging holds only `libfoo.so.8`, `-lfoo` does not fail; the
linker moves on through `-L` and finds the SDP's prebuilt `libfoo.so`. The build
succeeds and the binary uses a library this workspace is not allowed to use.
readline's `support/shlib-install` has no `nto-qnx` case and does exactly this.
`scripts/check-devlinks` now audits for it, flagging hardest when the SDP has a
library of the same name waiting to take over.

**50. A shared library can be built with unresolved symbols and nobody notices.**
Shared libraries are linked with undefined symbols allowed, so a build-system
bug that forgets a `target_link_libraries` produces a `.so` that installs
cleanly and only fails when an *executable* finally links it. libyuv compiles
its MJPEG decoder under `-DHAVE_JPEG` but links libjpeg into the `yuvconvert`
tool alone; SVT-AV1 referenced `cpuinfo_isa` from a cpuinfo build that had
excluded QNX and therefore never compiled it. `scripts/check-undefined` exists
for this class and should be run after every batch.

**51. GNU `as` in the SDP rejects assembly clang accepts.** libyuv's aarch64
inline assembly uses `mov v0.8h, v4.8h` (a valid alias, but the canonical form
takes only `.8b`/`.16b`) and `ld1 {v4.16b}, %3` with an `"m"` operand — GCC
prints that operand as `[x3, 80]`, which `ld1` has no addressing mode for.
Neither is wrong; both are clang-isms that upstream never had reason to
question. Multimedia projects with hand-written NEON are where this shows up.

**52. Bare `-z` linker options are not forwarded by `qcc`/`q++`.** SVT-AV1 adds
`-z noexecstack -z relro -z now` to `CMAKE_EXE_LINKER_FLAGS`; the driver
swallows `-z` and then chokes on its value, so **every `try_compile()` after
that point fails**. The visible symptom is a cascade of unrelated features
turning themselves off — `__builtin_expect` "unsupported", `find_package(Threads)`
failing, `-march=armv8.2-a+dotprod` "not accepted" — with no mention of `-z`
anywhere. Spell them `-Wl,-z,noexecstack`. The same applies to `-Xlinker`.

**53. Loadable modules are invisible to a DT_NEEDED walk.** GStreamer elements,
gdk-pixbuf loaders and similar plugins are `dlopen`ed, so a bundler that follows
`DT_NEEDED` packages none of them: the registry comes up with one plugin and
every `gst_element_factory_make()` returns NULL. The plugin directory, the
libraries *those* plugins need, and `gst-plugin-scanner` all have to be carried
explicitly (`tests/make-bundle.sh`), with `GST_PLUGIN_PATH`, `GST_REGISTRY` and
`GST_PLUGIN_SCANNER` set. The same applies to data a library reads at run time:
`share/terminfo` for ncurses, the XKB rules for libxkbcommon, fontconfig's
config.

**54. Symlink chains are more than one link deep.** `libncursesw.so` ->
`libncursesw.so.6` -> `libncursesw.so.6.5`. Copying the head and the
`readlink -f` result leaves the middle link dangling, and the loader reports
"Could not load library" naming a file that is *right there*. Copy every step.

**55. QNX 8.0 ships alsa-lib, not QSA.** Ports carrying QNX audio support
usually target QNX 6.x's `<sys/asoundlib.h>` and `snd_pcm_plugin_*`; SDP 8.0
ships a real alsa-lib 1.2.8 (`<alsa/asoundlib.h>`, `libasound.so.5`) with the
standard API. SDL2's `elseif(QNX)` selects its QSA backend and fails on the
missing header — the right answer is to let the generic ALSA backend win.

**56. `dbus_message_demarshal()` rejects a message that was never sent.**
Serial 0 is not a valid wire value, so a marshal/demarshal round trip on a
freshly constructed message fails with no useful error. Set a serial first.
Not QNX-specific, but it looks exactly like a byte-order porting bug.

**57. The QNX gcc spec injects its own `-rpath-link`, and it wins.** The driver
puts
`-rpath-link ${QNX_TARGET}/<arch>/lib/gcc/12.2.0:${QNX_TARGET}/<arch>/lib:${QNX_TARGET}/<arch>/usr/lib:${QNX_TARGET}/<arch>/opt/lib`
on the ld command line *before* anything you pass, and ld takes the first
match. So for any soname the SDP also ships — `libssl.so.3`, `libcrypto.so.3`,
`libxml2.so.2`, `libzstd.so.1`, `liblzma.so.5`, `libbz2.so.1`, `liblz4.so.1`,
`libncursesw.so.6` — a *transitive* `DT_NEEDED` resolves to the SDP's older
copy no matter how many `-L` or `-rpath-link` options of your own you add.
That is how libcurl's entirely valid call to `SSL_get0_group_name` came out as
an undefined reference: it was checked against the SDP's OpenSSL, not the 3.5.1
in staging. `-L` does not help, because `-L` is only consulted for explicit
`-lfoo`, not for a dependency's dependencies.

The fix is that ld expands the literal `${QNX_TARGET}` in that spec from the
*environment*. `env.sh` therefore builds a link sysroot at
`stage/<arch>/linkroot` — every SDP entry symlinked through, with
`<arch>/usr/lib` a real directory where our libraries overwrite the SDP's names
— and points `QNX_TARGET` at it, keeping the real path in `QNX_TARGET_SDP`.
The spec's own search then finds our copies first, and the SDP is never
modified. `scripts/qnxbuild` refreshes the tree after every install.

**58. `find_package()` will hand you the SDP's prebuilt library.** With the SDP
ahead of staging in `CMAKE_FIND_ROOT_PATH`, `find_package(ZLIB)` returned
`$QNX_TARGET/<arch>/usr/lib/libz.so` — and the SDP's zlib has soname
`libz.so.2` where upstream zlib builds `libz.so.1`. The binary then records
`libz.so.2` in `DT_NEEDED` and loads the SDP's copy on the target no matter
what `LD_LIBRARY_PATH` says. Eleven staged libraries were affected, with the
same pattern for c-ares (`libcares.so.8` vs `.so.2`), expat and ncurses.
Staging now comes first, and `scripts/check-sdp-leak` audits for it: it reports
any staged binary whose `DT_NEEDED` names a soname we do not provide but do
build. b2 and other hand-rolled build systems need telling separately —
Boost.Iostreams took `-sZLIB_INCLUDE=` / `-sZLIB_LIBPATH=`.

**59. `--enable-widec` renames every ncurses library.** A consumer asking for
`-lncurses` or `-ltinfo` finds nothing in staging and falls through to the
SDP's prebuilt ncurses, which is how gettext's tools ended up needing
`libncursesw.so.1` while we had staged `.so.6`. Distributions ship
compatibility symlinks; the recipe now does the same for `libncurses`,
`libform`, `libmenu`, `libpanel`, `libtinfo` and the matching `.pc` files.

**60. Prepended `-I` outranks a project's own include directories.** Putting
`-I<staging>/include` in `CMAKE_C_FLAGS_INIT` looks harmless until a project
vendors a library that is also installed in staging: protobuf and gRPC each
carry their own abseil, and on a rebuild their sources compiled against
whichever abseil headers were already installed — `'IsNegative' is not a member
of 'absl::numbers_internal'`. `-isystem` directories are searched after every
`-I`, so the project always sees its own copy first; the toolchain files use
that now.

**61. A comment inside a backslash continuation silently drops the assignments
before it.** This one was mine, in `scripts/qnxbuild`:

```sh
( cd "$B" && CC="qcc -V$T" CXX="q++ -V${T}_cxx" \
  CPPFLAGS="-I$STAGE/include" \
  CFLAGS="-I$STAGE/include" \
  # -rpath-link lets the linker resolve dependencies-of-dependencies
  LDFLAGS="-L$STAGE/lib" \
  "$S/configure" --host=... )
```

The comment ends the command. `CC`, `CXX`, `CPPFLAGS` and `CFLAGS` become plain
shell variables in the subshell — never exported — and `configure` runs with
`LDFLAGS` alone. It went unnoticed for 33 autotools packages because
`--host=<triple>` makes autoconf find the cross gcc by itself and pkg-config
supplied most include paths, so everything still built. The symptom finally
surfaced as zimg failing on `posix_memalign` with a `-D_QNX_SOURCE` that was
being set and thrown away. Keep the continuation unbroken and put the
commentary above it.

**62. `posix_memalign` is in `<malloc.h>` on QNX, and behind `__EXT_POSIX1_200112`.**
`<stdlib.h>` alone leaves it undeclared. The feature gate means `-std=c++14`
(which defines `__STRICT_ANSI__`) hides it even with the right header —
`-std=gnu++14` or `-D_QNX_SOURCE` restores it.

**63. Put `-D_QNX_SOURCE` on `CC`/`CXX`, not only on `CFLAGS`.** A configure
script may overwrite the flag variables outright — zimg sets
`CXXFLAGS="-fvisibility=hidden"` and discards everything passed in, and it
overwrites `CXX` too unless the variable is exported into its environment.
Carrying the define on the compiler variable survives the first; exporting it
properly (hazard #61) handles the second.

**64. QNX has a real `libregex.so` — do not stub it.** The compat stubs this
workspace ships for `librt`, `libdl`, `libpthread` and friends are empty
archives, because QNX genuinely folds those into libc. `libregex` is *not* one
of them: it exists in `$QNX_TARGET/<arch>/lib`. The empty `libregex.a` sat
earlier on every link line and shadowed it, so libedit linked with
`regcomp`/`regexec`/`regfree` unresolved. Check that a stub is actually needed
before adding one.

**65. `trietool`, `wayland-scanner`, `xgettext` — generators must be native.**
libthai builds its word-break dictionary by *running* `trietool`, which
libdatrie installs; the cross-built copy is a QNX binary. `configure` then
reports "You need trietool[-0.2] ... or just use --disable-dict", and taking
the `--disable-dict` escape would ship a libthai that cannot break Thai text.
`scripts/build-host-tools.sh` builds a native libdatrie for the tool alone.

**66. A shipped `./configure` can contain an unexpanded macro.** libdatrie and
libthai call `AX_COMPARE_VERSION` (autoconf-archive). Their released
`configure` carries the macro name through verbatim, and the *shell* cannot
parse it — `syntax error near unexpected token` — even with
`--disable-doxygen-doc`, because bash parses the whole block before deciding
not to run it. Regenerating is no help without the macro installed, so the
recipes patch `configure.ac` to drop the version comparison and then
`autoreconf`. These two also keep `build-aux` out of git, so automake's own
`config.guess`, `config.sub`, `compile`, `missing` and `install-sh` have to be
dropped in first.

**67. NASM output-format tables do not list QNX.** libass's configure maps
`$host_os` to a `-f` format and errors out on anything unlisted — "could not
identify NASM format for nto-qnx", with a suggestion to `--disable-asm` that
would quietly cost you every SIMD path in the subtitle renderer. QNX on x86 is
ELF with the SysV ABI's 16-byte stack alignment, i.e. exactly the `linux*`
entry. The list is simply older than anyone building libass for QNX; the same
shape of table appears in x264, dav1d and FFmpeg.

**68. A raw `ld` invocation sees only `LDFLAGS`.** ncurses links its shared
libraries by calling `ld` directly rather than through the compiler driver, so
none of the driver's built-in `-L` paths apply. Once `libregex.a` stopped
shadowing QNX's real `libregex.so` (hazard #64), `-lregex` failed there because
`$QNX_TARGET/<arch>/lib` was never on that command line. Conversely the bare
`-rpath-link <dir>` spelling that raw `ld` wants makes qcc reject configure's
own link test outright — no single spelling satisfies both, so the recipe names
the directory with `-L` and drops `-rpath-link` entirely.

**69. Three separate platform tables in CPython, and QNX is in none of them.**
`--enable-shared` is *accepted* and then quietly builds `libpython3.14.a`,
because `LDLIBRARY` is only set to a `.so` for hosts named in one case
statement; `CCSHARED` omits QNX too, so the core objects get no `-fPIC` and the
link dies on `R_AARCH64_ADR_PREL_PG_HI21 against symbol _PyRuntime`. Worse,
CPython *does* have a QNX case for `LINKCC` -- written for a native build, it
sets `LINKCC=qcc` with no `-V<target>`, so a cross build drives the default
target's linker and rejects its own objects ("Relocations in generic ELF
(EM: 183)"). An existing QNX case is not evidence that cross-building was ever
tried. Ten separate fixes were needed in all; see
`ports/python3/patches/0001-qnx-support.patch`.

**70. `posix_madvise` reports errors by returning them, `madvise` by errno.**
mimalloc (vendored inside CPython) calls `madvise`, which QNX does not have.
Substituting `posix_madvise` is not a rename: it returns the error number and
leaves errno alone, while mimalloc's callers retry on `EAGAIN` and fall back on
`EINVAL` by *reading errno*. The wrapper has to translate. Also, plain
`POSIX_MADV_DONTNEED` is advisory on QNX where mimalloc wants pages actually
released -- `POSIX_MADV_DISCARD_NP` is the faithful equivalent.

**71. SIMD alignment contracts are load-bearing, and only one architecture
tells you.** zimg documents "the image address and stride must be a multiple of
the alignment imposed by the host CPU architecture. On x86 and AMD64, this is 32
bytes." A plain `malloc` gives 16. On aarch64 the 16-byte NEON requirement is
met by accident and everything passes; on x86_64 the AVX kernels execute
aligned loads against a misaligned address and the process dies. Test both
architectures, and honour the alignment the API asks for rather than the one
that happens to work.

**72. X.Org's `autoreconf` needs util-macros on the build machine.** Every
X.Org `configure.ac` opens with `XORG_MACROS_VERSION`, so without
xorg-util-macros installed natively autoreconf stops with "must install
xorg-macros 1.8 or later" -- a build-machine dependency, not a target one.
`ACLOCAL_PATH` has to point at it.

**73. `malloc(0)` is a run-time probe, so cross configure cannot answer it.**
X.Org libraries ask, and guessing either way is a silent behaviour change.
QNX documents the answer: "If size is zero, then malloc() returns a valid
pointer with a size of zero" -- so `--disable-malloc0returnsnull`. Look the
answer up rather than picking the flag that makes configure proceed.

**74. A release tarball is not a slower git checkout.** GTK4's `meson.build`
says it outright: *"For git checkouts, but not for tarballs..."* -- the CSS
themes are compiled from Sass at build time, and if `sassc` is missing meson
builds it as a subproject and then tries to **run** it, which in a cross build
surfaces as an `exe_wrapper` requirement. Xfce is the same shape: a git tree
turns on maintainer mode and regenerates `*-aliasdef.c` with `xdt-csource`, a
tool 4.20 no longer ships. Switching six Xfce packages plus GTK4 to the release
tarballs fixed all of them and made two patches unnecessary. When a project
publishes tarballs, use them -- they *are* the supported source.

**75. `--default-library=both` can silently empty a shared library.** ThorVG
defines `TVG_STATIC` whenever a static library is part of the build, and
`TVG_STATIC` expands `TVG_API` to nothing. The `.so` builds, installs, and
exports zero `tvg_*` symbols. Any project whose visibility macro keys off a
static-build define behaves this way, and nothing before the first link against
it will tell you. `scripts/check-devlinks` now also reports a staged shared
library with no dynamic symbols.

**76. gnulib's relocation is Linux-only, and it fails silently.**
`relocatable.c::find_shared_library_fullname()` is implemented for Linux and
Cygwin and nothing else — both by scanning `/proc/self/maps`, which QNX does
not have in that format. On QNX the function compiles to nothing,
`get_shared_library_fullname()` always returns NULL, and `relocate()` hands
every path straight back unchanged. Nothing reports an error. Enchant made the
consequence visible: it `dlopen`s its spell-checking providers from a directory
composed from the configure-time `PKGLIBDIR`, so a broker installed anywhere
but the configured prefix constructed fine and enumerated **zero** providers.
`dladdr()` answers precisely the question the `/proc` scan asks — which object
contains this address — and QNX 8.0 has it in libc, with `Dl_info` exposed
under `__EXT_UNIX_MISC` (which `_QNX_SOURCE` turns on). Any package vendoring
gnulib's relocatable module and relying on it has the same hole.

---
## A note on `scripts/check-undefined`'s running time

It scans every staged shared library against the union of QNX's own exports and
ours. That union is ~1.1 M symbols, and it used to be re-sorted *inside* the
per-library loop — roughly a thousand sorts of a million lines. At 177 packages
that took about half an hour; by 205 it ran past a 30-minute timeout and looked
exactly like a hang. The merge is now done once, up front, and a full run is
about nine minutes per architecture. Still not fast, but finite and honest.

**77. A macro being defined is not evidence the kernel implements it.**
QNX's `<sys/ioctl.h>` defines `TIOCPKT` — `_IOW('t', 112, int)`, commented
"pty: set/clear packet mode" — but `devc-pty` does not implement the request
and returns `ENOTTY`. VTE enables pty packet mode under `#if defined(TIOCPKT)`
and treats failure as fatal, so `vte_pty_new_sync()` failed outright with
"Inappropriate I/O control operation". **Ignoring the errno would have been
worse than the failure**: `vte.cc`'s reader is compiled under the *same* `#if`
and strips a leading packet-header byte from every `read()`, so with packet
mode off but that branch compiled in, the first byte of real terminal output
would be eaten and misread as a header. A compile-time probe for a request the
kernel may not honour has to switch every site that depends on it, together.

**78. A staged target binary can shadow a host tool for every later build.**
Porting pkgconf put a QNX `pkgconf` in `stage/<arch>/usr/local/bin`. The CMake
toolchain sets `CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER` so host programs are
found normally — but `CMAKE_PREFIX_PATH` still contains the staging prefix, and
`find_program` searches `<prefix>/bin`. `find_package(PkgConfig)` matched the
QNX binary, tried to run it for its version, and reported **"Could NOT find
PkgConfig (missing: PKG_CONFIG_EXECUTABLE)"** — a message that names neither
the file it found nor why it rejected it. Every subsequent CMake package would
have hit this. The staging `bin` and `sbin` are now in `CMAKE_IGNORE_PATH`.

**79. Upstream merges change who owns a library.**
ATK was folded into at-spi2-core at 2.52, so at-spi2-core 2.54 installs
`libatk-1.0` and `atk.pc` itself. With a separate `atk` recipe (2.38) still in
the tree, both staged `libatk-1.0.so.0.*` and whichever built last won — the
ordering dependence a clean rebuild is supposed to eliminate. Nothing failed at
build time. It surfaced on target, because 2.54 makes `ATK_ROLE_PUSH_BUTTON` an
alias of `ATK_ROLE_BUTTON` and `atk_role_get_name()` started answering "button"
where 2.38 answered "push button". Before adding a package, check whether it now
contains something already ported.

---
## Running the suite on a small target

`tests/make-bundle.sh` produces about 170 MB, which unpacks to more. A default
`mkqnximage` VM has a **147 MB disk in total**, so the ordinary deploy dies
part-way through the copy with `scp: Connection closed` — which reads like a
network fault and is not one.

`tests/deploy-and-run-chunked.sh <arch> <host> [budget-MB]` handles that case.
For each test binary it resolves the shared libraries that binary actually
needs, packs tests into batches whose *union* of libraries fits the budget,
then copies, runs and deletes one batch at a time. It needs only the budget
free at any moment — 35 MB is comfortable on a 147 MB image — at the cost of
one ssh round trip per batch.

**80. QNX 8.0 has no System V shared memory, so X11's MIT-SHM cannot exist.**
libc exports `shm_open` and `shm_create_handle` but no `shmget`/`shmat`/`shmdt`
/`shmctl`, and there is no `<sys/shm.h>` (though `<sys/ipc.h>` is present, which
makes header probes for "SysV IPC" misleading). MIT-SHM hands the X server a
SysV segment id, so it is impossible here, not merely unported. cairo's Xlib
surface includes `<sys/shm.h>` unconditionally; it already had a complete
no-SHM stub for servers without the extension, and the patch takes that path
when the header is missing. cairo's XCB surface has the same dependency and is
left off.

**81. Vendored dependencies turn into duplicate libraries.** gRPC's CMake with
`PROVIDER=module` installs everything it bundles. Next to the protobuf recipe
that meant **two protobufs** (29.0 and 29.3) built against **two abseils**
(20240722 and 20240116 — 86 libraries each), the unversioned `lib*.so` symlinks
pointing at whichever installed last, overlapping `include/absl` and
`include/google` trees merged file by file, and the c-ares recipe's
`libcares.pc` and `libcares.a` silently replaced by gRPC's bundled c-ares. The
test binary for gRPC mapped both protobufs and both abseils into one process
and still passed — protobuf aborts when one descriptor registers twice, so that
was luck. The linker said only "may conflict with". There is now one abseil
recipe, protobuf and gRPC consume it and each other as packages, and
`scripts/check-dupes` fails if any library stem is staged under two sonames.
Same-soname overwrites (the c-ares case) it cannot see; install manifests can.

**82. protobuf 29 generated code compiles against exactly one runtime.**
`compiler/cpp/file.cc` emits `#if PROTOBUF_VERSION != <version>` into every
`.pb.h`, so the host `protoc` used by a cross build must be the same release as
the staged runtime — not whichever `protoc` a vendoring project brings along.

**83. With two headers of the same name under one prefix, `-isystem` order
decides.** Lua 5.4 installs `include/lua.h`; LuaJIT installs
`include/luajit-2.1/lua.h`. GCC ignores a `-I` that duplicates an `-isystem`
directory, which is why luv (given both) correctly picked LuaJIT's. A CMake
imported target, however, adds its include directory as another `-isystem`,
*after* the toolchain's, and the 5.4 header won: the test referenced
`lua_pcallk`, which LuaJIT does not have. Checking the consumer's undefined
symbols against the provider's exports is how to tell which header a library
was really compiled against.

**84. A cross build's generator must see the same target the compiler sees.**
LuaJIT generates its interpreter by running `buildvm` on the build machine.
For `TARGET_SYS` values its Makefile does not list — QNX among them — buildvm is
compiled with `LUAJIT_OS_OTHER`, while the target sources see `__QNX__` and get
`LUAJIT_OS_POSIX`. The two disagree on `LJ_HASPROFILE`, which adds fields to
the end of `jit_State`; `jit_State` precedes the dispatch table in `GG_State`;
and the assembler VM reaches global state by offsets from that table computed
with the *host's* layout. Every such access was 16 bytes off, and LuaJIT
crashed on `print(1)` — with the JIT on or off — inside `lj_tab_setstr`, on a
`lua_State` whose `base` and `top` were odd addresses. It built, linked and
passed every static audit. An earlier version of the recipe stated in a
comment that this host/target mismatch was harmless; it was the whole bug.

**85. An embedded interpreter will find somebody's standard library.**
`test_python` initialises CPython with an isolated config and no program
name, so CPython locates its prefix by searching `PATH` for `python3`. On the
Raspberry Pi image that is `/system/bin/python3` — QNX's own Python 3.14 — so
our `libpython` loaded QNX's stdlib and QNX's prebuilt `_ssl`, `_ctypes` and
`_sqlite3` from `/system/lib/python3.14/lib-dynload`, and the test's
"ssl / sqlite3 / ctypes through libffi" checks passed without touching our
ports. The x86_64 image has no system Python, so the same test failed there
with "Failed to import encodings module", which is what exposed it. The bundle
now carries our stdlib, the test sets `PyConfig.home` from the runner, and it
asserts that `os`, `_ssl`, `_ctypes`, `_sqlite3`, `zlib`, `_bz2`, `_lzma` and
`pyexpat` all load from inside the bundle.

**86. A fix that lives only in `src/` does not exist.** Converting the Xfce
recipes from git to release tarballs retired the patches that regenerated
sources — and, by mistake, the two `SA_RESTART` guards in libxfce4util and
xfconf as well. The trees in `src/` still carried the edits, so everything kept
building until a from-scratch rebuild re-fetched the sources. Auditing every old
checkout for modified files no recipe patch accounts for found no others.

**87. The clean rebuild is where undeclared dependencies surface.** At 218
packages it found three more: libXtst needs libXi, garcon needs libxfce4ui,
tumbler needs libxfce4util. Each built fine incrementally because the
dependency happened to be staged already. The driver also used to say nothing
when a package was skipped because a dependency failed, so one broken leaf hid
everything above it; it now prints `!! dependency failed`.

**88. An environment script that prepends is a slow leak.** `env.sh` and the
SDP's `qnxsdp-env.sh` both prepend to `PATH` every time they are sourced, and
the driver sources `env.sh` twice per package. At 218 packages `PATH` crossed
the kernel's 128 KB limit on a single environment string, after which *every*
`exec` failed with "Argument list too long". The first casualty was the
`dirname` that computes `WS`; it failed silently, `WS` became the current
directory (inside `src/iperf3`, where a `do_patch` hook had `cd`'d), and the
second half of the clean rebuild reported "no recipe" for 150 packages. The
previous clean rebuild, at 177 packages, stayed just under the limit, and an
incremental build never gets close. `env.sh` now records the caller's `PATH`
and `ACLOCAL_PATH` on first use and rebuilds from them, and refuses to continue
if `WS` does not contain `env.sh`.

**89. A clean rebuild is not optional, and it is not one-and-done.**
The rebuild at 218 packages (2026-09-18, ~4 h for both architectures) found,
in one pass: three undeclared dependencies (libXtst→libXi, garcon→libxfce4ui,
tumbler→libxfce4util), two patches that existed only as edits in `src/`
(hazard 86), a fourth undeclared dependency that was linking QNX's prebuilt
ncurses (aspell), a package whose feature detection had silently changed
underneath it (libev, hazard 90), and an environment leak that broke the run
itself (hazard 88). Every one of those built fine incrementally. The previous
clean rebuild, 41 packages earlier, had been clean.

**90. Fixing the build environment can change what configure finds.**
Teaching the autotools path to link QNX's split libc (`libepoll`, `libeventfd`,
`libsignalfd`, `libfsnotify`) fixed real link failures — and also made every
`AC_CHECK_FUNCS` probe for those APIs start succeeding. libev went from `poll()`
to epoll, and turned on an inotify path that does not compile on QNX. A
tree-wide diff of `config.h` against the previous build found five such changes
in 218 packages; the rest were gnulib's "is this math function in libc without
-lm" probes, now answered differently because `-lm` is always on the link line.
Worth running after any change to the shared flags: it is the only way to see
what else moved.

**91. A library you never staged cannot leak — so the audit never looked.**
`scripts/check-sdp-leak` flagged a DT_NEEDED resolving to the SDP only when the
same library *stem* was also staged. gettext was configured `--disable-nls`, so
it installed no `libintl` at all, and **56 staged libraries** — glib, GTK, ATK,
every Xfce component — linked QNX's prebuilt `libintl.so.1` instead. Invisible
for weeks, and exactly the thing this workspace exists to avoid. The audit now
works the other way round: everything loaded from the SDP must appear in an
explicit allowlist of genuine OS and toolchain libraries (libc, libc++,
libsocket and the split-libc pieces, libasound, libregex, QNX's own logging and
JSON), and anything else is reported — whether or not we happen to have staged
a copy. Re-run under that rule, the tree had exactly one offender.

**92. A cross build cannot answer a run-time question, and gettext's answer
changes its headers.** GNU gettext decides `HAVE_POSIX_PRINTF` by *running* a
probe. Cross-compiling, it assumes "no", and `<libintl.h>` then does
`#define printf libintl_printf`. Any C++ class with a `printf` member breaks if
it is declared before that header is included and called after it — which is
how WebKit's `WTF::PrintStream` failed with "has no member named
libintl_printf". QNX does support positional arguments; verified on target with
`snprintf(b, n, "%2$s-%1$s", "a", "b") == "b-a"`, so `gt_cv_func_printf_posix=yes`
is the truthful answer and the macros disappear. The SDP's own libintl.h has
`#if !0` at that guard; ours now has `#if !1`.

**93. Reserving address space is not reserving memory — but the target still
has to have it.** JSC reserves its structure heap with
`tryReserveUncommittedAligned(size, structureHeapAddressSize)`. On failure the
*size* halves, eight times over, but the *alignment* never does, so the default
4 GB value needs up to 8 GB of contiguous address space on every attempt. A QNX
VM with 1 GB of RAM cannot give that: all eight fail and a `RELEASE_ASSERT`
aborts inside `JSContextGroupCreate()`, before a line of JavaScript runs. The
same binary passed on an 8 GB Raspberry Pi, so it read as an architecture
difference and was really an address-space one. QNX now uses 128 MB, the value
PLAYSTATION already uses for the same reason.

**94. Test targets run out of disk, and it looks like broken ports.** A 61 MB
`/data` on a default mkqnximage VM filled with ~59 MB of scratch files from
previous runs. Every test that writes a file then failed, and the failure list
*grew* between runs (5, then 9 tests) while the ports were fine. Two smaller
traps came with it: QNX's filesystem does not report the space back until a
`sync` settles, so `df` still said 100% right after deleting everything; and my
first cleanup pass deleted `/tmp/qnx-port-tests-run.*`, which is the
`XDG_RUNTIME_DIR` the runner had just created, breaking every Wayland socket
with "unable to open lockfile". `run-on-target.sh` now clears only its own
leftovers, before and after, and warns when the filesystem is nearly full.

**95. `-D` on a CMake command line is not a compiler define.** WebKit reads
`STRUCTURE_HEAP_ADDRESS_SIZE_IN_MB` as a preprocessor macro in `StructureID.h`
and has no CMake plumbing to turn a cache variable into one. Passing
`-DSTRUCTURE_HEAP_ADDRESS_SIZE_IN_MB=128` to `cmake` therefore did nothing at
all, silently — the build succeeded and the target still aborted. Check that a
"knob" is wired to the thing you think it is; `grep` the build system for the
name before trusting it.

**96. WebKitGTK ports to QNX; a browser window does not.** WebKitGTK 2.44
builds and runs on QNX 8 — `libwebkit2gtk-4.1.so` (99 MB), the JS engine, the
Web and Network processes — and reports itself as
`Mozilla/5.0 (X11; QNX RaspberryPi5) AppleWebKit/605.1.15`. Seven fixes were
needed and **only two were about QNX at all**: CMake 4's stricter `if()`,
ICU 76's second C++ API switch, and a PID-socket argument GLib ports push
unconditionally while guarding its declaration, its fd hand-off *and* the
child's parsing with `OS(LINUX)`, are upstream problems that any current build
hits. What is genuinely QNX is the structure-heap reservation (hazard 93) and
this: **X11 and Wayland windowing cannot work.** QNX's only EGL belongs to
Screen and types `EGLNativeDisplayType` as an integer handle, not a display
pointer, so `eglGetDisplay(Display*)` does not even compile:

    invalid conversion from 'Display*' to 'EGLNativeDisplayType' {aka 'long unsigned int'}

Both windowing targets are therefore off (`ENABLE_X11_TARGET=OFF`,
`ENABLE_WAYLAND_TARGET=OFF`), which costs 2 of 1712 source files and leaves the
engine, DOM, layout, networking and media intact. Painting into a window would
need a `PlatformDisplay` backend written against QNX Screen — new development,
not a port, and honest to say so rather than ship something that links and
cannot draw.

**97. A patch added after the source is fetched is not applied.** The driver
applies `ports/<pkg>/patches/` on first fetch only. The structure-heap patch
was copied into the WebKitGTK recipe after its tree already existed, so the
build silently used the unpatched file and aborted on the 1 GB x86_64 target
exactly as JavaScriptCore had. `rm -rf src/<pkg>` after adding a patch, or
apply it by hand — and check the source, not the recipe directory, before
believing a fix is in.

**98. QNX's loader does not expand `$ORIGIN`.** A run path of
`$ORIGIN/../lib` -- DT_RPATH or DT_RUNPATH, with or without `-z origin` --
finds nothing on QNX 8.0 ("Could not load library"); an absolute run path
works. Components with a prefix of their own (`opt/<name>`) passed their tests
only because the test runner puts `opt/*/lib` on `LD_LIBRARY_PATH`; their
commands failed when started by hand. Give such a prefix's binaries the
absolute run path of where the package installs (`/usr/local/opt/<name>/lib`).

**99. QNX's C library crashes on `printf("%s", NULL)`.** glibc prints
`(null)`; QNX's libc dereferences it in strlen. Log lines that print an
optional string (labwc's theme name, swaybg's output description) crashed
programs that work everywhere else -- found only by running them.

**100. libc is the first `DT_NEEDED` of every executable, so it wins symbol
interposition.** qcc links `libc.so.6` ahead of everything else, and the
loader searches objects in that order. A library that defines libc or libm
names itself and calls them internally through the PLT runs QNX's copies
instead of its own: muslflt's `snprintf` called its `vsnprintf` and got QNX
libc's (`%a` of 1.0 printed `0x8p-3`, not musl's `0x1p+0`). Link such a
library with `-Wl,-Bsymbolic` so its internal calls bind inside it
(`ports/muslflt/recipe.sh`).

**101. `-export-dynamic` is often gated on the GNU compiler ID.** lighttpd
finds its built-in modules with `dlsym(RTLD_DEFAULT, "mod_x_plugin_init")`,
which needs the executable's symbols exported; its CMake adds
`-Wl,-export-dynamic` only when `CMAKE_C_COMPILER_ID` is GNU or Clang, and qcc
is `QCC` (hazard 4). The server then tried to `dlopen` `mod_indexfile.so`,
which is never built, and refused every configuration
(`ports/lighttpd/patches/0001-*`). A program that looks itself up with
`dlsym` needs `nm -D` to show those symbols.

**102. gnulib's `getcwd()` replacement walks `..` with `openat()` and
`fdopendir()` (hazards 25, 120).** A cross build cannot run gnulib's getcwd
probes, so it assumes the system getcwd is broken and replaces it with an
implementation that `openat(fd, "..", O_RDONLY)`s its way to the root, which
QNX refuses with `ENOTDIR`. gettext's `msgfmt -o` writes through
`open_supersede()`, which canonicalizes the output name starting from
`getcwd()`, so it could not write any file, even one in the current
directory: "error while opening ... for writing: Not a directory".
`xgettext -o` uses plain `fopen()` and worked, which hid the pattern.
gettext's getcwd now calls the system getcwd on QNX
(`ports/gettext/patches/0001-*`), and the recipe sets
`-DO_SEARCH=O_DIRECTORY` for the rest of gnulib. An earlier guess, that
`readlink()` returns another errno than EINVAL for a directory, was wrong.
Look for `REPLACE_GETCWD` = 1 in `config.status`.

**103. `fdopendir()` needs `O_DIRECTORY` as well (hazard 25).** A
recursive walker that `openat(parent, name, O_RDONLY)`s each subdirectory
and `fdopendir()`s it fails one level down. apk's `mkpkg` reported "failed
to process directory 'usr': -1" (`ports/apk-tools/patches/0004-*`).

**104. `--allow-shlib-undefined` hides missing `DT_NEEDED` entries.**
A shared library links even when it never names the libraries it calls.
libmetis.so had no NEEDED entry for libGKlib.so, and libGKlib.so had none for
libregex, where QNX keeps `regcomp` (it is not in libc). Every consumer had
to list the whole chain itself. Compare `readelf -d` of each library with its
undefined symbols (`ports/metis/patches/0002-*`, `ports/gklib/patches/0002-*`).

**105. Build-machine interpreter paths leak into installed scripts.**
CMake's `find_program()` and autoconf's `@SHELL@` both run on the build
machine. check's `checkmk` began with `#! <stage>/linkroot/.../usr/bin/awk -f`
and the fakeroot wrapper with `#!/bin/bash`; a QNX target has neither path.
Set the interpreter explicitly (`-DAWK_PATH=/usr/local/bin/awk`), declare
the runtime dependency, and check the `#!` line of every installed script.

**106. The unversioned `lib*.so` goes to the -dev package.** Code that
dlopens or `LD_PRELOAD`s a library by its unversioned name works in the
staging tree and fails on an installed target. The fakeroot wrapper preloaded
`libfakeroot.so`, which ships in fakeroot-dev; the runtime file is
`libfakeroot-0.so` (`ports/fakeroot/recipe.sh`).

**107. `std::filesystem::current_path()` can throw.** QNX's libc++
sizes its getcwd buffer with `pathconf(".", _PC_PATH_MAX)` and throws
`filesystem_error` when that fails. An uncaught call aborts the program.
The likely cause of openal-info dying with SIGABRT while enumerating HRTF
data files (not yet confirmed on the target). Use the `error_code` overload
(`ports/openal-soft/patches/0001-*`).

**108. "Minimal plugin set" switches drop security features.**
strongSwan's `--disable-defaults` also removed the `constraints` plugin, so
X.509 name, path-length and policy constraints were not enforced, and with
it the `revocation` plugin and the `pki` tool. The build and the load checks
stayed clean. Only strongSwan's own `certnames` unit suite (0/10) showed it.
When disabling defaults, list what they were and re-enable the security-
relevant ones explicitly (`ports/strongswan/recipe.sh`).

**109. A trylock is no way to ask whether a mutex is held.** libmetal's QNX
`metal_mutex_is_acquired()` probed with `pthread_mutex_trylock()`: on a free
mutex the probe *took* the lock (the next acquire deadlocked), and from the
thread holding the mutex QNX's trylock did not fail with `EBUSY`, so a held
mutex read as free. Read the owner word of the mutex's `sync_t` instead
(`_NTO_SYNC_MUTEX_FREE` is free; `_NTO_SYNC_INITIALIZER`/`_DESTROYED` are
markers, also free). The deeper bug was in the same header:
`metal_assert(pthread_mutex_unlock(mutex) == EOK)` -- `metal_assert()` is
`assert()`, so with `NDEBUG` (every Release build) lock, unlock, init and
`pthread_cond_init` were compiled out and libmetal's mutexes did nothing.
Never put the call itself inside an assert. `ports/libmetal/patches/0002`
fixes both, and `metal_get_timestamp()` (it computed the time and returned
0). Found by the run-level test, not by the build.

**110. Perl's versionless `site_perl` is not in `@INC`.** The perl port has
an empty `inc_version_list`, so `SITELIB_STEM` (`lib/perl5/site_perl`) is
never searched; only `site_perl/<version>` and its arch directory are.
perl-error installed `Error.pm` into the stem, and `perl -MError` failed on
every target. Pure-Perl modules go into `site_perl/<perl VERSION>`, as
perl-mozilla-ca does (`ports/perl-error/recipe.sh`).

**111. ctypes loaders that name `lib*.so` (hazard 106) hide in Python.**
ntpsec's `ntp.ntpc` loaded `libntpc.so` -- the development symlink in
ntpsec-dev -- so with only ntpsec installed every `ntp.*` import, and with it
ntpdig, ntpq and ntpkeygen, failed with "Can't find ntpc library". Grep
Python packages for `CDLL`/`find_library` and try the runtime soname first
(`ports/ntpsec/patches/0002`).

**112. CPython's `shutil.rmtree()` walks the tree with directory fds opened
without `O_DIRECTORY` (hazards 25, 103).** `_rmtree_safe_fd` does
`os.open(name, O_RDONLY | O_NONBLOCK, dir_fd=...)` and then `os.scandir(fd)`
and `*at()` calls under it, which QNX refuses with `ENOTDIR`. setuptools'
`bdist_wheel` stops removing its egg-info ("[Errno 20] Not a directory:
'build/bdist.qnx-.../qnxdemo-1.2.3-py3.14.egg-info'"). The fix belongs in the
python3 port: `ports/python3/patches/0002` adds `O_DIRECTORY` to those opens
(and to `os.fwalk`'s); `test_python3_setuptools` checks `shutil.rmtree`
directly.

**113. A test that links a static library by `-l<name>` is not relinked when
the library changes.** CMake/ninja track `${LIB}/lib<name>.a` as a dependency
only when it is named by path. cpuinfo's fix was in the rebuilt
`libcpuinfo.a`, yet `test_cpuinfo` kept the old code (a static library has no
run-time package to update either). Name static libraries by path in
`port_test()` (`tests/run-level/g5-other.cmake`).

**113. A C++ shared library linked by the C driver has no NEEDED for
libc++ (hazard 104 again).** FreeImage's `Makefile.gnu` links
`libfreeimage.so` with `$(CC)` and upstream's `-lstdc++`; the recipe had
replaced that with `-lm`, so the library recorded no C++ runtime at all. It
loaded only into C++ programs, and a C program linking it failed on
`__cxa_*` and `operator new`. Name QNX's runtime explicitly (`-lc++`,
`ports/freeimage/recipe.sh`). The same check found more libraries calling
libraries they do not record: libmetis-gtsam (libregex, `ports/gtsam/patches/0001`),
libarm_compute (libregex, `ports/computelibrary/patches/0002`) and
librpi_camera_ipa (libslog2, libyaml; `ports/libcamera/files/CMakeLists.txt`).

**114. `strings` can invent paths in x86_64 binaries.** GCC on x86_64
builds a short constant string such as `GIO_MODULE_DIR`
(`/usr/local/lib/gio/modules`) from two overlapping 16-byte SSE stores, so
`.rodata.cst16` holds `/usr/local/lib/g` and `lib/gio/modules`, and
`strings` runs them together as `/usr/local/lib/glib/gio/modules`. That path
exists nowhere; the aarch64le build of the same file shows the real one. To
audit a compiled-in path, check the `-D` in `build.ninja` or the aarch64le
binary, or `objdump -s -j .rodata.cst16` the object, not `strings` on x86_64.
It cost a glib rebuild here.

**115. Header-only portability bugs show up only in consumers.** A recipe
flag fixes the library's own compile but not the installed header. farmhash
was built with `-D__LITTLE_ENDIAN__`, yet its `farmhash.h` compares glibc's
`__BYTE_ORDER` (undefined on QNX, so 0) and then includes `<endian.h>`, which
QNX does not have, so every program including it failed. Patch the header
(`ports/farmhash/patches/0003`, which uses the compiler's `__BYTE_ORDER__`).
Upstream header bugs belong here too: aws-c-cal's `hkdf.h` reused `ecc.h`'s
include guard, and including both lost one (`ports/aws-c-cal/patches/0001`).

**116. QNX's aarch64 `FE_*` rounding constants are not musl's or glibc's.**
QNX's `<aarch64/fenv.h>` defines `FE_UPWARD` 1, `FE_DOWNWARD` 2 and
`FE_TOWARDZERO` 3, the FPCR RMode field without its shift of 22. musl
(and glibc) use the bits in place (`0x400000`, `0x800000`, `0xc00000`). On
x86_64 the two agree. muslflt ships no headers, so its users pass QNX's
values: its `fesetround(FE_UPWARD)` returned -1 on aarch64 only. Its aarch64
`bits/fenv.h` and `fenv.s` now take and return QNX's values
(`ports/muslflt/patches/0001-*`).

**117. A call to a function that exists nowhere surfaces only at load time**
(the toolchain's `--allow-shlib-undefined`, hazard 104).
XNNPACK calls `cpuinfo_has_arm_sme()`, which is newer than the cpuinfo built
here. GCC 12 only warns about the implicit declaration, the library linked,
and every aarch64 program using it failed in the loader. Grep build logs for
`implicit declaration`, and run `nm -D <lib> | grep ' U '` against what is
staged (`ports/xnnpack/patches/0002-*`).

**118. `daemon()` without `noclose` closes every descriptor, not only 0-2.**
QNX implements it with `procmgr_daemon()`, which closes all open files
unless `PROCMGR_DAEMON_NOCLOSE` is given; Linux and the BSDs only point
0-2 at `/dev/null`. A server that forks, keeps a socketpair to its parent and
then calls `daemon(1, 0)` loses the socketpair: tmux's client reported
"server exited unexpectedly" and never created its socket, so tmux, and with
it every test that drives a program in a tmux pane, failed. Use
`daemon(nochdir, 1)` and redirect 0-2 yourself (`ports/tmux/patches/0002-*`).

**119. The main thread's stack is small; big arrays on it crash.** Programs
that keep large buffers on the stack -- ninja's `DepsLog::Load` declares a
512 KiB record buffer, its build-log reader 256 KiB -- die with "Memory
fault" on QNX before doing anything, while tools that never reach that
function work, which makes it look like a logic bug. Link with
`-Wl,-z,stack-size=8388608` (`ports/ninja/patches/0002-*`; BusyBox already
needed 4 MiB). METIS's `gpmetis` and `ndmetis` declared a 1.28 MB
`char filename[MAXLINE]` to build their output file name, and crashed only
when they wrote a result: `gpmetis -nooutput` worked
(`ports/metis/patches/0003-*`).

**120. gnulib's `O_SEARCH` fallback is `O_RDONLY`, so `save_cwd()` opens
`.` without `O_DIRECTORY` (hazard 25).** QNX has no `O_SEARCH`; gnulib then
defines it as `O_RDONLY`, and every `fchdir()`/`*at()` on that descriptor
fails with `ENOTDIR`. GNU tar died "failed to return to initial working
directory: Not a directory" on every extraction; gettext's `msgfmt` cannot
write its output file ("error while opening ... for writing: Not a
directory") for the same reason. `-DO_SEARCH=O_DIRECTORY` fixes all users
of the module (`ports/tar/recipe.sh`; gettext still needs it).

**121. QNX's `/bin/sh` (pdksh) has no POSIX character classes in patterns.**
`case "$f" in [[:alpha:]]*-*)` never matches, so xdg-desktop-menu refused
every file name as having no vendor prefix. Spell classes out (`[a-zA-Z]`)
in shell scripts that run on the target (`ports/xdg-utils/patches/0002-*`).

**122. LuaJIT's FFI resolves `ffi.C.<name>` in the executable, which must
export its symbols.** sysbench's configure adds `-rdynamic` only on Linux and
FreeBSD; without it every Lua script stopped with "Symbol not found" at its
first `ffi.C` reference. Link such programs with `-Wl,--export-dynamic`
(`ports/sysbench/recipe.sh`).

**123. `setrlimit(RLIMIT_FSIZE)` always fails with `EINVAL`**, even with the
values `getrlimit()` just returned. sudo raises that limit at start-up and
printed "setrlimit(rlimit_fsize): Invalid argument" on every run
(`ports/sudo/patches/0003-*`).

**124. `/etc` is not the package root's `etc`.** Packages install under the
apk root (`/data/oss`), so a program built with `--sysconfdir=/etc` reads
the base system's `/etc` and never sees the configuration its own package
ships: sudo found no `/etc/sudoers`. Configure with
`--sysconfdir=/usr/local/etc` (`ports/sudo/recipe.sh`).

**125. qnxpkg's dev split takes CMake's own data directory.** The pattern
`^usr/local/share/cmake[^/]*/` (CMake package files of libraries) also
matches `share/cmake-4.1`, CMake's Modules: the cmake package ships without
them and every cmake run stops "Could not find CMAKE_ROOT". The pattern
now keeps `share/cmake-<version>/` in the runtime package (scripts/qnxpkg).

**118. The SDP's `libbacktrace.so.1` needs `libunwind-nto.so.0`, which
target images often lack.** Both are in the SDP, but a vanilla QNX 8.0 image
ships libbacktrace without its unwinder. g3log's crash handler linked
`-lbacktrace` (`ports/g3log/patches/0001`), so every program using g3log
failed at start with "Could not load library libunwind-nto.so.0", only for a
stack dump printed on a crash. Don't link optional crash-time helpers:
dlopen them when needed (`ports/g3log/patches/0002`). `check-sdp-leak`
allowlists libbacktrace; its dependency is the part that is missing.

**126. A Python extension module must not link libpython.** libxml2's CMake
links its `libxml2mod` with `Python::Python`, which puts `libpython3.14.so`
in its NEEDED list -- and since the module ships in the libxml2 package,
qnxpkg would make libxml2 (and everything that uses it) depend on python3.
CPython's own `lib-dynload` modules take the C API from the interpreter that
loads them; link `Python::Module` instead, and drop the version suffix so the
file is the `libxml2mod.so` that `import` looks for
(`ports/libxml2/patches/0001-*`).

**127. A mutex locked in a `pthread_atfork()` prepare handler cannot be
unlocked in the child.** The child's thread is not the owner on QNX:
`pthread_mutex_unlock()` there returns `EPERM`. Threaded perl locks
`PL_perlio_mutex` and `PL_op_mutex` before every fork and unlocks them in
both processes, so every backtick, `system()` and pipe open panicked in the
child ("panic: MUTEX_UNLOCK (1) [util.c:2877]") -- and autoconf/automake hung
for good. Re-initialise such mutexes in the child handler instead
(`ports/perl/patches/0002-*`); check any other code that follows the classic
lock-in-prepare/unlock-in-child pattern.

**128. `pkg-config --variable` returns sysroot-prefixed paths in a cross
build.** (First filed as a second number 114.) X.Org's `XORG_FONTROOTDIR` takes fontutil's `fontrootdir` from
pkg-config, and with `PKG_CONFIG_SYSROOT_DIR` set that is
`<workspace>/stage/<arch>/usr/local/share/fonts/X11`. libfontenc compiled it
in as its encodings directory, so on a target `FontEncDirectory()` pointed
into the build machine and no encoding file was found. (A target that still
had an older correct build hid it on x86_64.) Pass the target path
explicitly: `--with-fontrootdir=$PREFIX/share/fonts/X11`
(`ports/fontenc/recipe.sh`). `strings` on the staged library shows it.

The cause is general, not fontenc's. The build machine's pkg-config is
pkgconf, and with `PKG_CONFIG_SYSROOT_DIR` set pkgconf puts the sysroot in
front of *every* path it prints, `--variable` answers included;
freedesktop.org's pkg-config, which upstream build files are written for,
prefixes only the `-I`/`-L` of `--cflags`/`--libs`, and a variable meant for
the build machine says so itself with `${pc_sysrootdir}` (wayland-protocols'
`pkgdatadir`, xcb-proto's `xcbincludedir`). configure scripts and meson.build
files take the answers as the target's directories: xkbcomp compiled in
xkeyboard-config's `xkb_base`, Xwayland fontutil's `fontrootdir` as its
font path, GTK `x11`'s and iso-codes' `prefix`, gst-plugins-base gio's
`giomoduledir`/`libdir`/`prefix` and iso-codes', Epiphany iso-codes' -- all
as `<workspace>/stage/<arch>/usr/local/...`. `toolchain/bin/pkg-config` now
wraps pkgconf (env.sh puts it first on PATH and in `PKG_CONFIG`, the meson
cross files name it): for `--variable` it sets
`PKG_CONFIG_FDO_SYSROOT_RULES=1`, so the answer is `/usr/local/...`;
`--cflags`/`--libs` are unchanged. Exceptions keep the prefix: variables
naming a program the build runs (`glib_mkenums`, `gdbus_codegen`,
`g_ir_scanner`, ...), and every query from CMake, whose FindPkgConfig fills
`<P>_INCLUDEDIR`/`_LIBDIR`/`_PREFIX` from `--variable` and projects compile
with them (the toolchain files set `QNX_PKG_CONFIG_VARIABLES=sysroot`; a
recipe can export the same). Check a compiled-in path in the build files
(`config.h`, `build.ninja`): on x86_64 `strings` showed GTK's
`/usr/local/share/X11/locale` as `/usr/local/sharehare/X11/locale`
(hazard 114).

**129. Packages must not name the build machine, but staging must.** Files
in stage/ are read by later cross builds there: a CMake package file, a
`*-config` script or Python's sysconfig data has to name the staging tree.
The packages get the target's paths instead, at packaging time:
`scripts/qnxpkg` passes every text file (no NUL byte) through
`scripts/lib/hostpaths.py`: `<any prefix>/stage/<arch>/usr/local` becomes
`/usr/local`; `-I`/`-L` into the link sysroot or the SDP and
`-Wl,-rpath-link` into staging are dropped; any other link-sysroot/SDP path
becomes the target's (`linkroot/<arch>/lib` -> `/lib`; in CMake files an SDP
`.../lib/libm.so` becomes `m`); the SDP's cross compiler is named, not
located (`qcc`); `#!` lines naming a build-machine interpreter get the
target's (`/usr/local/bin/python3`, `/usr/local/bin/perl`, `/bin/sh`); ROS 2's
colcon install tree (`build/<arch>/ros2/install`) becomes
`/usr/local/opt/ros/jazzy`; Python's sysconfig loses the hosttools python and
`-I`/`-L` into the source tree. Earlier workspaces of this machine are
listed in `scripts/lib/build-prefixes` -- packages built there still name
them. Two kinds of file are not rewritten: frozen m4 state (`*.m4f`, every
string length-prefixed) and binary files; and a `.pc` naming a hosttools/
program (wayland's `wayland-scanner.pc`) is not packaged at all. The package
fingerprint covers the rewritten bytes, so a package whose contents change
gets a new release. `scripts/check-artifacts` reads pkgroot the way qnxpkg
packages it and fails on what is left (classes A-C,
`scripts/lib/check-hostpaths.py`); harmless or not-yet-fixed leftovers are
listed in `scripts/lib/hostpath-allow`.

**130. A compiled-in data path can point into a build tree, too.** VLC's
contribs are installed into `build/<arch>/vlc/<triple>/` and built with that
as their prefix: the bundled fontconfig in libvlccore's subtitle renderers
looks for `<build>/vlc/<triple>/etc/fonts` and
`<build>/vlc/<triple>/var/cache/fontconfig`, gettext for
`<build>/vlc/<triple>/share/locale`. No staging tree is involved, so
neither the pkg-config fix nor the package sanitizer reaches it; the contribs
need target directories (`--sysconfdir=/usr/local/etc`, `--localstatedir=/var`,
`--localedir`) in their configure. Open (listed in hostpath-allow).

**131. For qcc, `-P` means "preprocess to a file", not "no line markers".**
`qcc -E -P x.c` writes `x.i` and prints nothing. perl-cross probes with
`$cpp -P` and reads stdout, and perl's `Errno_pm.PL` adds `-P` for GCC 5 and
later: with qcc, perl got no signal names ("No such signal: SIGHUP") and an
empty `%!` (`$!{EEXIST}` false, so autom4te died "cannot create
autom4te.cache: File exists"). perl-cross also asked qcc for `gccversion`
and got "0.0". Preprocess with the cross-GCC driver (`<triple>-gcc -E`) and
pass the real `gccversion` (`ports/perl/recipe.sh`).

**132. QNX's `getopt_long` does not restart on `optind = 0`.** It is
BSD-derived: `optind = 0` is the GNU spelling of "start over", which QNX
does not honour, so a first parsing pass sees no options at all. RetroArch
parses its arguments twice and handles `--config` only in the first pass:
the option was silently ignored. Restart with `optind = 1; optreset = 1`
(`ports/retroarch/patches/0003-*`).

**133. Files in an IFS image (`/proc/boot`) carry no section headers.**
Tools that work from sections -- `scanelf`'s soname and symbol search, `nm`
-- report nothing for `/proc/boot/libc.so.6`, while header-only queries
(ELF type) still work. Use a library from a package to test such tools.

**134. D-Bus EXTERNAL authentication: libdbus expects SCM_CREDS, GDBus
sends none (hazard 40).** QNX's socket layer is FreeBSD's, so libdbus is
configured with `HAVE_CMSGCRED` and reads the peer uid only from an
`SCM_CREDS` message the client must send. libdbus clients send one; GLib has
no credentials type for QNX (`gcredentialsprivate.h` knows `__FreeBSD__`, not
`__QNX__`), so GDBus clients send the bare nul byte, dbus-daemon learns no
uid -- and worse, with no GCredentials support GDBus's EXTERNAL client sends
`g_credentials_get_unix_user()`'s failure value, uid -1, as its identity, which
no server accepts: "Exhausted all available authentication mechanisms (tried:
EXTERNAL)" -- xfconf, and with it every Xfce program, could not reach the
session bus. GLib now treats QNX like FreeBSD (`struct cmsgcred` sent as
`SCM_CREDS`, `ports/glib/patches/0002-*`), and libdbus falls back to
`getpeereid()` when a peer sends no credentials (`ports/dbus/patches/0001-*`).

**135. "How many file descriptors can there be?" has no answer on QNX in
code that knows only Linux, glibc and the BSDs.** VTE's `fdwalk()` fallback
(copied from GLib) asks `prlimit()`/`getrlimit()` only under `__linux__` or
`__GLIBC__` and `sysconf(_SC_OPEN_MAX)` only on the BSDs and macOS; anything
else gets `RLIM_INFINITY`, and `fdwalk()` then refuses with `ENFILE`. VTE
marks descriptors close-on-exec that way before every spawn, so no terminal
child ever started: xfce4-terminal opened a window with no shell in it.
Add `__QNX__` to the `sysconf()` branch (`ports/vte/patches/0004-*`).

**136. QNX's inotify emulation (`/dev/fsnotify`) can block a single-threaded
event loop.** During D-Bus service activation dbus-daemon was found blocked
in the fsnotify server (state REPLY on `/dev/fsnotify`, its inotify fd), from
its service-directory watcher (`bus/dir-watch-inotify.c`), while the
activated xfconfd waited for the daemon to answer its authentication: a
deadlock, and every activated service timed out. (At first it looked like
the exec-error pipe; it is not.) dbus is now built with
`-Dinotify=disabled -Dkqueue=disabled`: the default watcher does nothing,
and the daemon reads the service directories at start and again on a cache
miss (`bus/activation.c`, `update_service_cache`) (`ports/dbus/recipe.sh`).
Treat inotify-based watchers on QNX as suspect in any event loop.