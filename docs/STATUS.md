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
3. **libunwind**: QNX's repository has it; the SDP already ships
   libunwind.so.8 (+ -nto, -coredump). A port must not ship a second
   libunwind.so.8 (`check-dupes`, `check-sdp-leak`): decide between
   documenting the SDP's and a prefixed build.
4. **Compare with QNX's lists and port what is missing** (`docs/upstream-comparison.md`
   is out of date): lists in `tmp/qnxlists/` are scratch; regenerate from
   github.com/qnx-ports, github.com/qnx-ports/build-files and
   repo.oss.qnx.com (8.0.5 APKINDEX). About 89 origins of QNX's repository
   are not here (labwc, wlroots, swaybg, linux-headers-input-codes are now).
5. **Compiled-in build-host paths**: some binaries carry the staging path --
   mostly harmless RPATH entries and assert file names, but a few are data
   paths (epiphany's iso-codes and locale directory; check's checkmk
   shebang). libxkbcommon 1.8.1 had one (fixed with -Dxkb-config-root).
   A check for it belongs in `check-all`.
6. qnxpkg does not bump `-r` when only a package's dependencies change
   (fingerprints cover contents): mind it for published packages.
7. Components still not verified on a target are published later with
   `publish-github-release --changed` after `verify-on-target --changed`.

## Machine notes

- Targets used: vanilla QEMU VMs made by `vanilla-vm/make-vm` (`~/vm/oss-vanilla-<arch>`).
- `hosttools/` is machine-specific (paths compiled in): rebuild it on a new PC.
- Moving to another PC: `scripts/workspace-pack` here, `workspace-setup --from` there
  (`docs/build-environment.md`).
