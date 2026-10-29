# vanilla-vm -- every port on a vanilla QNX 8.0 QEMU image, in one go

Scripts that build a stock QNX image with `mkqnximage` (nothing from this
repository inside it), boot it in QEMU, set it up for apk and then install
and test the ported components the way a user would get them: from this
machine's `dists/` (built here) or from the GitHub releases of
github.com/openqnx/oss.

| Script | What it does |
|---|---|
| `run-all [x86_64\|aarch64le\|both] [--from local\|github] [--together] [--only "<names>"]` | image, apk, then every component installed and tested -- one at a time (default; a 4 GB `/data` is enough), or `--together`: all installed, then the whole test suite. Results in `tests/results/<arch>.tsv`, transcript `logs/vanilla-vm-<arch>.log` |
| `desktop [x86_64\|aarch64le] [--from local\|github] [--no-apps]` | the image **with graphics** in a QEMU window: QNX Screen, the `oss-desktop` session (labwc on wlroots' QNX Screen backend, background, Xfce panel, terminal, file manager) and every ported application with a desktop entry, in its menu |
| `screenshot [arch] [file.png]` | a PNG of the desktop VM's screen |
| `make-vm <arch> [--graphics] [--data-size MB] [--rebuild]` | only build and boot an image; prints its address |
| `stop-vm [arch\|both] [--desktop] [--wipe]` | stop (or delete) the VMs |

```bash
export QNX_SDP_ROOT=~/qnx800                 # SDP 8.0 (mkqnximage, QEMU on this machine)
vanilla-vm/run-all both                      # x86_64 and aarch64le in parallel, local dists/
vanilla-vm/run-all x86_64 --from github      # what a user installs from GitHub
vanilla-vm/desktop x86_64                    # the desktop, in a window
```

## In the desktop window

Right click (or Super+Space) on the background: the menu -- Terminal,
Files, *Applications* (every installed application), *Run the port tests*
(the test suite, in a terminal). Super+Return opens a terminal, Super+E the
file manager, Alt+Tab switches windows. On the target, `oss-desktop` starts
the session again; `oss-desktop --headless` starts it without a display.

## What it needs

- QNX SDP 8.0 with `mkqnximage` and QEMU (`qemu-system-x86_64`,
  `qemu-system-aarch64`) on a Linux host, the `virbr0` bridge QEMU's
  bridge helper may use (libvirt's default network), KVM for x86_64.
- For `desktop`: QNX's **Screen Board Support VirtIO** package in the SDP
  (the virtio GPU driver for Screen; `mkqnximage --graphics=yes` stops with
  that name when it is missing -- install it with the QNX Software Center),
  and a desktop session on this machine for the QEMU window.
- aarch64 runs emulated (no KVM): correct, several times slower.

VM directories: `~/vm/oss-vanilla-<arch>` and `~/vm/oss-desktop-<arch>`
(`VM_ROOT` to change). The SSH key the images are built with is
`tests/ssh/qnx-target` (made on first use, not in git).
