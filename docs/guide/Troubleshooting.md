# Troubleshooting

| Symptom | Cause / fix |
|---|---|
| `curl: (60) SSL certificate problem` | `SSL_CERT_FILE` is not set ([Base QEMU images](Base-QEMU-Images), sections 4 ① and 5) |
| `ldd: ... libeventfd.so.1 not found` (or another QNX library) | the QNX run-time libraries are missing ([Base QEMU images](Base-QEMU-Images) 4 ②), or `/data/oss/qnx-runtime/lib` is not in `LD_LIBRARY_PATH` |
| a program cannot find its data files, fonts show as boxes | the `/usr/local` link is missing: `ln -sP /data/oss/usr/local /usr/local` (after every reboot) |
| `No space left on device` | `/data` is too small: make the image again with the sizes in [Base QEMU images](Base-QEMU-Images) 2 |
| apk: `UNTRUSTED signature` | the key file must be named `openqnx-oss-20261029.rsa.pub` ([Install with apk](Install-with-apk) 2) |
| apk: `sdl2 ... conflicts: sdl2-compat` | install only one of the two |
| `apk add` says "1 error", `….trigger: exited with error 127` | a target set up before 2026-11-03: apk lacks `--force-no-chroot`. Run `sed -i 's#--root /data/oss#--root /data/oss --force-no-chroot#' /data/oss/bin/apk; apk fix`, or add the option to your alias |
| GTK program: "No GSettings schemas are installed" | installed without apk: `glib-compile-schemas /usr/local/share/glib-2.0/schemas` |
| `404` from curl | the release does not match the name's first letter: a-l → `…-<ARCH>`, m-z → `…-<ARCH>-m-z` |
| labwc: "Can't open 'dri' library" | `dri_gbm.so` from the SDP is missing in `/data/oss/qnx-runtime/lib` ([Base QEMU images](Base-QEMU-Images) 4 ②) |
| graphics image, x86_64: `screen` crashes, "No virgl contexts available" | QEMU runs without KVM: the user must be in group `kvm` and `/dev/kvm` writable |
| aarch64 is slow | QEMU emulation; normal. Big packages (llvm, webkitgtk, pytorch) take minutes to install |

Still stuck? Open an [issue](https://github.com/openqnx/oss/issues) with the command, its output and
`uname -a` of the target.
