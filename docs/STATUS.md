# Where the work stands

Read this when picking the work up (on this PC or another one). Keep it
current: it is the hand-over, not a history (that is `git log`).

## Done

- 676 components ported for QNX SDP 8.0, x86_64 and aarch64le; recipes and
  QNX patches in `ports/`, original + patched sources in `ported/`.
- Packages (`dists/8.0/<arch>`, apk v2, signed with `keys/private/openqnx-oss-20261029.rsa`)
  and port tests (`test_suites/8.0/<arch>`); full suites 210/210 on both architectures.
- Checked on vanilla mkqnximage QEMU images (x86_64 and aarch64le) through apk,
  from local `dists/` and from the GitHub releases (`tests/results/<arch>.tsv`).
- Published: `main` on github.com/openqnx/oss; releases `packages-8.0-<arch>`,
  `apk-8.0-<arch>`, `tests-8.0-<arch>` hold the verified packages
  (`scripts/publish-github-release --verified --upload`).
- **Releases split by package name** (2026-11, published): `packages-8.0-aarch64` needed 1025 assets, GitHub allows 1000
  per release, and every version is kept. The apk and packages releases now
  hold the names 0-9, a-l (the old tag names, so existing URLs keep working
  for that half) and m-z in new `-m-z` releases (`apk-8.0-<arch>-m-z`,
  `packages-8.0-<arch>-m-z`), each a complete repository with its own signed
  index. Rule: `scripts/lib/shards.py`. apk-setup writes four repository
  lines, oss-get merges several INDEX files.
- **Tests release split too** (2026-11-02, published):
  `tests-8.0-aarch64` needed 1175 assets. `tests-8.0-<arch>` (the old tag)
  now holds `qnx-test-<name>` with `<name>` 0-9, a-l, `tests-8.0-<arch>-m-z`
  the m-z ones and the runner `qnx-port-tests` (sorted as `port-tests`, in
  that shard only), each with its own `APKINDEX.tar.gz` and `INDEX`.
  Staged with `--verified`: aarch64 apk/packages a-l 720/719, m-z 549/548,
  tests a-l 675, m-z 503; x86_64 apk/packages a-l 713/712, m-z 553/552,
  tests a-l 671, m-z 503 assets per release.
  Published with `--upload --prune`: the -m-z releases were created first,
  then the m-z files that moved were removed from the old ones. Older
  revisions of m-z tests stay on the old tests release, unindexed. Targets
  set up before need the m-z lines (INSTALL.md, docs/packages.md).
- **Batch of ~80 more components** (2026-11-01/02): libraries
  (apr, poco, folly, rocksdb, log4cxx, ...), services and tools (redis, ntp,
  tcpdump, inetutils, e2fsprogs, cups-filters, toybox, ...), mesa (llvmpipe,
  lavapipe; in `opt/mesa`, renamed libraries so the SDP's EGL/GLES stay in
  use), robotics/simulation (cartographer, bullet3, SIL Kit, ioquake3),
  Python (boto3, lxml, matplotlib, ...), GNOME (libadwaita, libpanel). Each
  passed its run-level test on both QEMU targets; the QNX problems the tests
  found are hazards 137-151 in docs/porting-hazards.md.
- **Looked at, not ported:**
  - ndash: the public source (google/ndash, 2017) bundles a 2016 Chromium
    base with much Linux-only code; nothing names it in the SDP.
  - p-net: current public releases ship only the stack core (no build
    files, no OS layer); the last complete release supports Linux only, so
    QNX would need a new network and OS abstraction.
  - valgrind: SDP 8.0 ships valgrind 3.17 (memcheck works on the QEMU
    target; pass `--extra-debuginfo-path` with libc's .sym file); a newer
    upstream needs a new OS port.
  - Go / etcd: upstream Go has no QNX target; the one public fork covers
    32-bit QNX 6.5 only. A qnx/amd64 + qnx/arm64 port is several weeks.
  - Flutter: no public QNX engine/embedder; the Dart VM would need a port.
  - perf: QNX has no perf_event_open. can-utils: built without the AF_CAN
    tools (QNX has no SocketCAN).

## In progress / next

0. **QNX-list gap** (2026-11-04/05, `reports/gap/qnx-oss-gap.tsv`, local): what QNX's
   package lists have and this repository lacked. Batches run in parallel by agents
   (`lab/agents/PORT-AGENT-RULES.txt`), each a run-level group `tests/run-level/gN-*`.
   - Committed: g7 build tools, g8 math, g9 media/graphics, g10 net/system, g11 perl,
     g12 languages, g14 python-a, g13 RViz2 (ros2-rviz, OGRE GLES2 on QNX Screen) +
     assimp (176 components); g15 robotics (+ ros2 fixes), g16 python-b, g17 Qt6 modules
     (qt6 rebuilt with Wayland, r2) (68 components). verify-on-target: 244/244 PASS on
     x86_64 and on aarch64 (2026-11-10). The earlier aarch64 failures were the VM being
     unreachable or slow; moveit2's test now plans again (up to 3 times) when the
     randomised OMPL path is rejected by ValidateSolution.
   - Published 2026-11-11: the 244 components' packages and tests
     (`publish-github-release --verified --changed --upload`), `main`, the package site
     and the Wiki. `PUBLISH_OUT=build/release scripts/check-release-sync`: 12 releases,
     0 differ, 0 not uploaded. Pitfall: publish-github-release stages in `build/release`,
     while check-release-sync and gen-pkg-site read `tmp/release-staging` by default --
     pass `PUBLISH_OUT=build/release` / `gen-pkg-site -r build/release`, then
     `deploy-pkg-site --no-gen` (a stale staging tree gave a site without the new ports).
   - The work moved to another PC on 2026-11-10 (`scripts/workspace-pack`); the SDP there
     is `~/claude-work/qnx800`, cmake 3.28.3 comes from `uv tool install` (`~/.local/bin`).
     Pitfall: `TEST_GROUPS` takes full group names (`g15-robotics`, not `g15`); a wrong
     name rebuilds only the base tests, shortens the staged `tests.list` and bumps
     `qnx-test-ceres` (its binary is not reproducible).
   - `scripts/check-all` on 2026-11-10: only the four known findings (avahi man links,
     libfolly.pc, mesa's GLES libraries on aarch64le, libhailort's libpci). A fifth, new on
     this PC, is fixed: lanelet2 1.2.3-r2 no longer installs the source tree's path
     (mrt_cmake_modules), and overlay builds (`scripts/lib/ros2-overlay.sh`) name the
     venv's Python (CMake took `/usr/bin/python3.12` here, which has no catkin_pkg).
   - Next after that: a native compiler on the target like QNX's Raspberry Pi 5 image
     (clang + binutils + make/cmake there). llvm 22.1.8 (clang, lld) is ported but only
     tested with `clang -c`; missing: QNX headers/crt/link libraries on the target (copied
     from the user's SDP by target-bootstrap), cc/gcc wrappers and clang config, C++ on the
     SDP's libc++.so.2 ABI, and a compile-link-run test.
   - Then SocketCAN: QNX 8.0 has no AF_CAN, so can-utils lacks candump/cansend/cangen.
     Look for a public SocketCAN layer for QNX 8 first; otherwise a compatibility library
     over QNX's dev-can drivers plus a virtual CAN bus for tests; rebuild can-utils.
   - Then the CAN ecosystem on top of it: python-can, cantools, canmatrix, opendbc,
     CANopenNode, ros2_canopen, isotp-c, can-isotp, udsoncan, python-OBD, J1939,
     libcanard (OpenCyphal), socketcand, cannelloni, libsocketcan, Wireshark/tshark,
     SavvyCAN, Cangaroo, Scapy.
   - Left: heavy ones (bazel, openjdk, ROCm, xla, tvm, mediapipe, jax, ...) need a
     feasibility check; then the 158 version updates (`scripts/check-updates`).
   - Known check-all failures (before this work): avahi man links, libfolly.pc,
     mesa embedded libc, hailort libpci. Found and not fixed: readline lacks a
     DT_NEEDED on ncurses (tgoto/tgetent), python3 strptime without LANG
     (hazard 204), RViz GLSL shaders (no point clouds / picking on GLES).

1. **`vanilla-vm/`** (done, in use): `run-all` builds vanilla QEMU images, bootstraps
   apk and installs + tests every component. x86_64 full run: 419 passed; the
   failures it found were checker bugs (fixed in `verify-on-target`: apk-tools
   removed after its own check, 644 libraries, `.o` files, QNX ldd's TLS
   teardown error) and test data (`TEST_EXTRAS`). aarch64 full run: in progress.
2. **The desktop** (`oss-desktop`, `vanilla-vm/desktop`): wlroots 0.20.2 with QNX's
   QNX Screen backend, labwc 0.20.2, swaybg, libsfdo, libdrm-headers,
   linux-headers-input-codes are ported and tested headless on the x86_64
   vanilla image (test_wlroots, test_labwc incl. swaybg, test_libsfdo pass;
   `oss-desktop --headless` brings labwc + swaybg + xfce4-panel up). Updated
   for it: wayland 1.24.0, wayland-protocols 1.47, pixman 0.46.4,
   libxkbcommon 1.8.1 (older versions kept). **Blocked on graphics:** the
   SDP here lacks QNX's "Screen Board Support VirtIO" package that
   `mkqnximage --graphics=yes` needs -- install it (QNX Software Center),
   then `vanilla-vm/desktop x86_64` and check the session on screen
   (xfconfd D-Bus activation, panel menu, every application).
   Not yet packaged/verified for aarch64 (waits for the aarch64 run).
2b. **Remote display for VirtualGL / RViz** (done, 2026-10-30): RViz rendered on a
   Linux machine (vglrun, e.g. an Ubuntu guest) shown on QNX -- over VNC:
   libvncserver (+ SDLvncviewer, straight onto QNX Screen) and gtk-vnc
   (gvncviewer on the desktop); over VirtualGL's X11 transport: xwayland
   (an X display on labwc) and virtualgl (vglclient, vglconnect; client only,
   no desktop GL on QNX). test_virtualgl runs labwc -> Xwayland -> vglclient
   and sends a frame over the VGL protocol, read back from the X server.
   Not yet seen on a screen (needs the graphics image).
3. **libunwind** (done): upstream 1.8.3 in its own prefix, /usr/local/opt/libunwind
   (the SDP's 1.8.2 stays the system libunwind.so.8); test_libunwind passes on
   both architectures.
4. **QNX's lists, ported so far** (2026-10-30/31): the X.Org libraries libxcvt,
   libxshmfence, libxkbfile, libice, libsm, libxt, libxmu, libxfont2, xkbcomp,
   xcb-util; ed, patch, lzip, wget, htop and btop (QNX's QNX platforms), libmd,
   opusfile, capstone, libaec, miniz, lcm, mimalloc and pistache (QNX's
   changes), mcap-cpp, four tree-sitter grammars, 21 pure-Python modules
   (pytest, tornado, curio, click, ...), whisper.cpp and stable-diffusion.cpp
   (QNX's changes; own prefixes), ncnn, mujoco, OpenTTD's free base sets,
   adwaita-icon-theme, docbook-xsl, perl-mozilla-ca. Each has a test with
   known answers. Not yet: doxygen, poppler, graphicsmagick (in progress),
   gnupg (+ libassuan, libksba, npth), nginx, glfw and raylib (now possible on
   the Wayland stack), asciidoc, perl-io-tty, dhcpcd, nss/nspr, the LLVM
   subprojects (lld, openmp, mlir, flang, compiler-rt), openrc, matplotlib,
   ml-dtypes, colcon, and QNX's own components (qnx-posix-publish-subscribe,
   libsysv-ipc-shim). Aliases already here: clang/llvm22 (llvm),
   libxml2.13 (libxml2), ros2-jazzy (ros2), vte3 (vte), webkit2gtk-4.1
   (webkitgtk), python3-tz (python3-pytz), python3-pandas (pandas).
5. **Compiled-in build-host paths** (2026-10-31): the cause of the data
   paths into stage/ was pkgconf's sysroot on `--variable` answers; fixed in
   `toolchain/bin/pkg-config` (porting-hazards 128) and xkbcomp, xwayland,
   gtk3, gst-plugins-base, epiphany rebuilt (both arches, not yet
   repackaged). Text files are sanitized at packaging (`scripts/qnxpkg`,
   `scripts/lib/hostpaths.py`, hazard 129) -- the next `qnxpkg` run gives the
   affected packages a new `-r` (about 30 components, ROS 2 among them).
   `check-artifacts` now fails on classes A-C in pkgroot as packaged; open
   items are in `scripts/lib/hostpath-allow` (qt6's host-Qt wrapper scripts
   and CMake files, onnx's and gtsam's CMake files, VLC's contrib
   fontconfig/gettext directories -- hazard 130). Not yet verified on a
   target.
6. **`$ORIGIN` is not expanded by QNX's loader** (porting-hazards 98):
   tensorflow-lite (94 files) and pytorch (4) rely on it and work only with
   their lib directory on LD_LIBRARY_PATH; give them absolute run paths.
7. Package trees are now checked for build-machine paths (check-artifacts):
   ncurses, geany-plugins and gflags had shipped them (fixed, -r1), several
   -dev .pc files named the old workspace (fixed). qnxpkg compares legacy
   index entries (no fingerprint) by archive contents now: it had rewritten
   published r0 names locally (repaired: those became r1).
8. qnxpkg bumps `-r` when only a package's dependencies change (same
   files, other RUNTIME_DEPS): fixed 2026-10-31, libxfce4ui -> hicolor-icon-theme.
9. Components still not verified on a target are published later with
   `publish-github-release --changed` after `verify-on-target --changed`.
10. qnxpkg kept the release number of unchanged packages but wrote their
   archives again (new gzip/tar times = new bytes under the same name);
   `--changed` does not upload names already there, while the uploaded
   indexes described the new bytes -- 683 GitHub assets stopped matching
   their index (aarch64 `zstd` failed to install). Fixed: an unchanged
   entry keeps its files and index line as they are (`unchanged()`), and
   the release assets were re-uploaded. Check after every publish: every
   asset's GitHub `digest` equals the staged file's sha256.

11. **Package site** (`scripts/gen-pkg-site` -> `tmp/pkg-site/`, static, ~34 MB):
   a pkgs.alpinelinux.org-style browser built from the staged release
   indexes (= GitHub's, checked by digest), with each package's files,
   dependencies both ways, QNX patches, license (`ports/<name>/LICENSE.id`,
   `scripts/detect-licenses`; 436 of 525 found, the rest "see upstream")
   and its on-target test log. Live at https://openqnx.github.io/oss/
   (branch gh-pages): run `scripts/deploy-pkg-site` after every publish.

## Machine notes

- Targets used: vanilla QEMU VMs made by `vanilla-vm/make-vm` (`~/vm/oss-vanilla-<arch>`).
- `hosttools/` is machine-specific (paths compiled in): rebuild it on a new PC.
- Moving to another PC: `scripts/workspace-pack` here, `workspace-setup --from` there
  (`docs/build-environment.md`).
