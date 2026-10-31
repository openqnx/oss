# Where the work stands

Read this when picking the work up (on this PC or another one). Keep it
current: it is the hand-over, not a history (that is `git log`).

## Done

- 451 components ported for QNX SDP 8.0, x86_64 and aarch64le; recipes and
  QNX patches in `ports/`, original + patched sources in `ported/`.
- Packages (`dists/8.0/<arch>`, apk v2, signed with `keys/private/openqnx-oss-20261029.rsa`)
  and port tests (`test_suites/8.0/<arch>`); full suites 210/210 on both architectures.
- Checked on vanilla mkqnximage QEMU images (x86_64 and aarch64le) through apk,
  from local `dists/` and from the GitHub releases (`tests/results/<arch>.tsv`).
- Published: `main` on github.com/openqnx/oss; releases `packages-8.0-<arch>`,
  `apk-8.0-<arch>`, `tests-8.0-<arch>` hold the verified packages
  (`scripts/publish-github-release --verified --upload`).

## In progress / next

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
5. **Compiled-in build-host paths**: some binaries carry the staging path --
   mostly harmless RPATH entries and assert file names, but a few are data
   paths (epiphany's iso-codes and locale directory; check's checkmk
   shebang). libxkbcommon 1.8.1 had one (fixed with -Dxkb-config-root).
   A check for it belongs in `check-all`.
6. **`$ORIGIN` is not expanded by QNX's loader** (porting-hazards 98):
   tensorflow-lite (94 files) and pytorch (4) rely on it and work only with
   their lib directory on LD_LIBRARY_PATH; give them absolute run paths.
7. Package trees are now checked for build-machine paths (check-artifacts):
   ncurses, geany-plugins and gflags had shipped them (fixed, -r1), several
   -dev .pc files named the old workspace (fixed). qnxpkg compares legacy
   index entries (no fingerprint) by archive contents now: it had rewritten
   published r0 names locally (repaired: those became r1).
8. qnxpkg does not bump `-r` when only a package's dependencies change
   (fingerprints cover contents): mind it for published packages.
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

## Machine notes

- Targets used: vanilla QEMU VMs made by `vanilla-vm/make-vm` (`~/vm/oss-vanilla-<arch>`).
- `hosttools/` is machine-specific (paths compiled in): rebuild it on a new PC.
- Moving to another PC: `scripts/workspace-pack` here, `workspace-setup --from` there
  (`docs/build-environment.md`).
