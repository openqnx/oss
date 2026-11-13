# A target image that uses the ports first

How to build a QNX 8.0 target image in which QNX supplies only what the system
needs to boot and run, and everything else comes from the components ported
here. `scripts/make-ported-image` builds such an image for QEMU; what was
tried and what was not is in "What the trial showed" below. The steps still
marked "to verify" have not been tried.

The starting point is the image `mkqnximage` makes (the lab's vanilla VMs,
`docs/qemu-test-vms.md`). Its file lists are in
`lab/vm/oss-vanilla-<arch>/output/build/{ifs,system,data}.build`; the tables
below were made from them (SDP 8.0.6, x86_64).

## In short

```bash
source ./env.sh x86_64                    # the SDP of the image, docs/sdp-versions.md
scripts/make-ported-image                 # the base set; or name the packages
cd lab/vm/oss-ported-x86_64 && mkqnximage --run=-h && mkqnximage --getip
ssh -i tests/ssh/qnx-target root@<ip>     # the ported sshd, the ported bash
```

The packages must be built and packaged first (`dists/<sdp>/<arch>/`).

## What the trial showed

One image, x86_64, SDP 8.0, QEMU, the base set (44 packages with their
dependencies, 147 MB under `/usr/local`; system partition 254 MB used):

- boots; the log-in over ssh is the ported `sshd` (port 22) into the ported
  `bash`; `ls`, `cat`, `find`, `sed` are the ported toybox; `grep`, `tar`,
  `awk`, `vim`, `less`, `curl`, `sqlite3`, `ping`, `htop`, `sudo`, `gdb` the
  ported tools; `curl https://` works with the ported OpenSSL and CA bundle.
- the ports need QNX libraries the stock image does not carry (`libepoll`,
  `libeventfd`, `libsignalfd`, `libtimerfd`): `make-image-root` lists them
  (`QNX-RUNTIME`) and the image script adds the missing ones to `/system/lib`.
- symbolic links in the unpacked tree are kept by `[-followlink]`.
- QNX's `sshd`, `dhcpcd`, `gdbserver`, `qconn` are still started by
  `startup.sh`; `sshd` is replaced at the end of the boot, the others are
  not yet.
- still QNX's because no port puts the name in `PATH`: `sh` (the `ksh` of
  the IFS), `openssl` (the command; the library is the port's), `top`.

Not tried: aarch64le, SDP 8.0.6, a board, removing QNX's duplicates from the
system partition (step 8), the port tests on this image, time
synchronisation (`chronyd` is in the image, not started), PAM.

## The idea: three layers

| layer | where | from | when it is available |
|---|---|---|---|
| 1. boot | the IFS (`/proc/boot`) | QNX only | from power-on |
| 2. base system | system partition (`/system`) | QNX, cut down to what has no port | once the disk is mounted |
| 3. everything else | `/usr/local` (system or data partition) | the ports | once the disk is mounted |

Layer 1 cannot use a port: nothing ported is on a disk the kernel can read
before `devb-*` and the file system are running, and what runs there must not
depend on `/usr/local`. This is why the toybox of QNX stays in the IFS even
though toybox is ported: the start-up script needs `ls`, `cat`, `ln`, `mount`
before the disk is there. After the disk is mounted, `PATH` and
`LD_LIBRARY_PATH` name `/usr/local` first, so every command typed, every
service started later and every program run is the ported one.

## Layer 1: what stays QNX in the IFS

Keep the IFS as `mkqnximage` makes it. Nothing here has a replacement:

| what | files | why |
|---|---|---|
| kernel, start-up | `procnto-smp-instr`, `startup-*` | the OS |
| C library, loader | `libc.so.6`, `ldqnx-64.so.2`, `libm.so.3`, `libgcc_s.so.1`, `libsocket.so.4` | every program, QNX's and ours, links these |
| logging | `slogger2`, `libslog2*.so`, `slog2info` | QNX service |
| PCI | `pci-server`, `lib/dll/pci/*`, `libpci*.so` | drivers |
| disk, file systems | `devb-eide` (or the board's), `io-blk.so`, `cam-disk.so`, `fs-qnx6.so`, `fs-dos.so`, `libcam.so.2`, `fsevmgr` | to reach the disk |
| console | `devc-ser8250` (or the board's) | serial console |
| security | `libsecpol.so.1`, `libjail.so.1`, `libqh.so.1`, `libqcrypto.so.1.0`, `qcrypto-openssl-3.so`, `libcrypto.so.3` | QNX services (random, secpol) load them |
| start-up script tools | `ksh`, `toybox` (+ its links), `on`/`waitfor`, `mount`, `umount`, `getconf`/`setconf`, `rtc`, `pidin`, `setfacl` | the script itself |

`libcrypto.so.3`, `libz.so.2` and `libjson.so.1` in the IFS are there for QNX
programs. They stay; the ports bring their own copies (below).

## Layer 2: what stays QNX on the system partition

Resource managers, drivers and the QNX-only tools that talk to them:

- services: `random`, `pipe`, `mqueue`, `devc-pty`, `dumper`, `io-sock` with
  `mods-*.so` and the network drivers (`devs-*.so`), `io-usb-otg` with
  `devu-*.so`, `libusbdi.so.2`, `devb-umass`, `devb-loopback`, `fs-nfs3`,
  `qconn`
- network configuration of io-sock: `ifconfig`, `route`, `sysctl`, `arp`,
  `ndp`, `pfctl`, `if_up`, `ifwatchd`, `netstat`, `sockstat`
- QNX system tools: `pidin`, `slay`, `use`, `hogs`, `shutdown`, `slog2info`,
  `tracelogger`, `pci-tool`, `usb`, `fdisk`, `mkdosfs`, `chkqnx6fs`, `df`,
  `ldd`, `pdebug`, `secpolmonitor`, `getfacl`
- the C++ run time of the toolchain: `libc++.so.2`, `libstdc++.so.6`,
  `libunwind.so.8`, `libcatalog.so.1`, `librcheck.so.1`, `libtracelog.so.1`,
  `librpc.so.2`. The ports are compiled with the SDP's compiler and link its
  `libc++`; it is part of the platform, like `libc`.
- log-in: `login`, `su`, `passwd` and the PAM modules (`pam_qnx.so`, ...),
  as long as the accounts are QNX's (see "Open points").

## Layer 3: what the ports replace

Everything in the image's system partition that has a port. Left column: what
`mkqnximage` puts in; right: the port to install instead.

| in the QNX image | port |
|---|---|
| `toybox` and its ~110 command links (after boot) | `toybox`, or `busybox`; `grep`, `tar`, `file`, `xz`, `bzip2` for the full tools |
| `ksh` as the log-in shell | `bash` or `zsh` (the IFS keeps `ksh` for the start-up script) |
| `sshd`, `ssh`, `scp`, `ssh-keygen`, `sftp-server` | `openssh` |
| `openssl`, `libssl.so.3`, `libcrypto.so.3` (system partition copy) | `openssl` |
| `curl`, `libcurl`, `libcares` | `curl`, `c-ares` |
| `sqlite3` | `sqlite` |
| `gawk` / `awk` | `gawk` |
| `less` | `less` |
| `vi` | `vim` |
| `bsdtar` | `libarchive` |
| `gdbserver` | `gdb` |
| `dhcpcd` | `dhcpcd` |
| `ntpd`, `ntpq`, `ntpdate`, `sntp`, `ntp-keygen` | `ntp`, or `chrony` |
| `tcpdump` | `tcpdump` |
| `iperf2`, `iperf3`, `libiperf` | `iperf2`, `iperf3` |
| `top` | `htop` |
| `ping`, `traceroute`, `hostname` | `inetutils` (also `telnet`, `ftp`, `whois`) |
| `libncursesw`, `libpanelw`, `libedit`, `libexpat`, `libxml2`, `libbz2`, `liblzma`, `libiconv`, `libintl`, `libxo`, `libfdt`, `libpam` | `ncurses`, `libedit`, `expat`, `libxml2`, `bzip2`, `xz`, `libiconv`, `gettext`, `libxo`, `libfdt`, `openpam` |
| the time zone data | `tzdata` |
| root with a password | `sudo` |

Which port owns a given file: `scripts/gen-dependency-index` output and
`docs/packages.md`.

## Two copies of one library

QNX's loader searches `LD_LIBRARY_PATH` before a program's RPATH, so with
`/usr/local/lib` first every program -- a remaining QNX one too -- gets the
ported copy when the soname is the same.

| case | libraries | what to do |
|---|---|---|
| different soname | `libz` (QNX `.so.2`, port `.so.1`), `libncursesw` (`.so.1` / `.so.6`), `libxml2` (`.so.16` / `.so.2`), `libexpat` (`.so.2` / `.so.1`), `libiconv` (`.so.1` / `.so.2`), `libintl` (`.so.1` / `.so.12`), `libjson` (`.so.1` / `.so.1.0.0`) | nothing: both can be present; each program finds its own. Keep QNX's copy only while a QNX program in the image needs it (`ldd` / `readelf -d` on what stays). |
| same soname | `libcrypto.so.3`, `libssl.so.3`, `libbz2.so.1`, `liblzma.so.5`, `libedit.so.0`, `libpam.so.2`, `libxo.so.0`, `libiperf.so.0`, `libfdt.so.1` | one copy wins for everybody. Leave QNX's copy out of the system partition and let the port's serve the QNX programs that remain -- **to verify** per library on a VM (the `qcrypto` plug-in with the ported `libcrypto` in particular). Until verified, keep QNX's copy in `/proc/boot` (IFS programs started before the switch keep using it) and start QNX services with their own `LD_LIBRARY_PATH=/proc/boot:/system/lib`. |

Never put a ported library under `/proc/boot` or `/system/lib`: the two trees
stay apart, and `scripts/check-dupes` keeps one soname per tree.

## Procedure with mkqnximage (a VM, or to try the layout)

`scripts/make-ported-image` does steps 2 to 7 and 9's build; the steps are
written out for a board, or to change what the script does.

1. **Build and package** the ports for the SDP of the image
   (`docs/sdp-versions.md`): `scripts/qnxbuild`, `scripts/qnxpkg --all`.
2. **Choose the base set.** A start:
   `toybox bash openssh openssl curl ca-certificates sqlite gawk grep less vim
   tar xz bzip2 file libarchive ncurses dhcpcd chrony tcpdump iperf3 htop gdb
   sudo tzdata apk-tools` -- plus whatever the product needs. Dependencies
   come with them.
3. **Make the root of layer 3 on the build host**:
   `scripts/make-image-root <arch> <package>...` unpacks the chosen packages
   and their dependencies into `build/image-root/<arch>/usr/local/...`, and
   writes `PACKAGES` (what is in it) and `QNX-RUNTIME` (the SDP libraries
   those programs need). The apk database is not made: packages added on
   the target later go to the data partition (`docs/target-guide.md`).
   toybox is packaged without command links; `make-ported-image` links its
   commands in `/usr/local/opt/toybox/bin`, which comes after
   `/usr/local/bin` in `PATH` -- the full tools win, toybox fills the rest.
4. **Create the image directory** as for the lab VMs, with room for the
   ports:

   ```bash
   . $QNX_SDP_ROOT/qnxsdp-env.sh
   mkdir -p lab/vm/oss-ported-x86_64 && cd lab/vm/oss-ported-x86_64
   mkqnximage --type=qemu --arch=x86_64 --hostname=ported-x86 \
       --ssh-ident=$WS/tests/ssh/qnx-target.pub \
       --sys-size=1500 --sys-inodes=60000 --data-size=4000 --data-inodes=80000
   ```

5. **Add layer 3** in `local/snippets/system_files.custom` (the lines go into
   `system.build`; the system partition is mounted at `/system`):

   ```
   # the ports: /system/usr/local, seen as /usr/local
   [uid=0 gid=0 dperms=755 type=dir] usr
   usr/local = /abs/path/to/build/image-root/x86_64/usr/local
   ```

   and make `/usr/local` point at it in `local/snippets/post_start.custom`
   (as the lab does for `/data/oss/usr/local`):

   ```
   ln -sP /system/usr/local /usr/local
   ```

6. **Put the ports first** in `local/snippets/profile.custom`:

   ```
   export PATH=/usr/local/bin:/usr/local/sbin:/usr/local/opt/toybox/bin:/system/bin:/proc/boot
   export LD_LIBRARY_PATH=/usr/local/lib:/system/lib:/system/lib/dll:/proc/boot
   export SSL_CERT_FILE=/usr/local/ssl/cert.pem
   ```

   The profile is read by log-in shells only; the ported `sshd` gets the
   same values in its configuration (`SetEnv`), and root's log-in shell is
   set to `/usr/local/bin/bash` in `/data/var/etc/passwd`.

7. **Start the ported services instead of QNX's.** `mkqnximage` starts QNX's
   `sshd`, `dhcpcd` and `gdbserver` from its own `startup.sh`, and has no
   option to leave them out (`mkqnximage --options` lists what there is).
   Until the templates are edited (step 8), replace them at the end of the
   boot, in `post_start.custom`:

   ```
   slay -f sshd
   /usr/local/sbin/sshd -f /system/etc/ssh/sshd_config.ported
   ```

   The ported `sshd` needs `/var/empty` (data partition) and a
   configuration that names the image's host keys
   (`/data/var/ssh/ssh_host_*_key`).

   The network must be up before `post_start` for this; `io-sock` and
   `ifconfig` stay QNX's.
8. **Take QNX's duplicates out of the system partition.** `mkqnximage` writes
   `system.build` from the templates in
   `$QNX_HOST/../common/mkqnximage/inputs/`; the entries of the "Layer 3"
   table are the ones to drop. Two ways, **both to verify**:
   - leave them in and only shadow them through `PATH` (step 6): nothing
     breaks, the image is larger than it needs to be, and QNX's copies are
     still there under `/system/bin`. Do this first.
   - then remove them: a copy of the templates edited for this image, given
     to mkqnximage as an extra content directory (`--extra-dirs=<dir>`,
     `mkqnximage --help=extra-dirs`), or post-process
     `output/build/system.build` and rebuild the partition
     (`mkqnximage --build=s`). Remove in small groups and boot after each.
9. **Build, boot, check**:

   ```bash
   mkqnximage --build --run=-h ; mkqnximage --getip
   ssh root@<ip> 'which sh ls ssh curl; ldd $(which curl) | head; pidin ar | grep -c /usr/local'
   scripts/verify-on-target ...      # the port tests, as on any target
   ```

   What to look at: every command resolves to `/usr/local`; no program
   reports a missing library; `use -i` on a running service names the ported
   build.

## A board (BSP image) instead of a VM

The same three layers, written by hand in the BSP's build files:

- the IFS build file of the BSP: leave drivers, `procnto`, `libc`, the
  start-up script tools; do not add anything ported.
- the file system image (`mkqnx6fsimg` build file, or what the BSP uses):
  the "Layer 2" list from the SDP (`[search=$QNX_TARGET/<arch>]`), and the
  directory of step 3 as `/usr/local`.
- the start-up script: after `mount`, set `PATH`/`LD_LIBRARY_PATH` as in
  step 6, then start the ported services.

For a board that already runs a QNX image, nothing has to be rebuilt to try
the layout: `scripts/target-bootstrap` + `apk add` put the same packages
under `/data/oss/usr/local` (`docs/target-guide.md`); the image step only
moves them into the image and removes QNX's duplicates.

## Open points

- **Accounts and PAM**: `login`, `su`, `passwd` use QNX's PAM modules
  (`pam_qnx.so`). Replacing them means `openpam` plus `sudo`/a ported
  `login`, with the account database kept compatible. Not tried.
- **Start-up script shell**: the IFS script is `ksh` syntax run by QNX's
  `ksh`. It could become a script for the ported `bash` only if `bash` and
  its libraries were put in the IFS, which the first rule forbids. Leave it.
- **Same-soname libraries with QNX programs** (table above): verify each.
- **Slimming the IFS**: `libjson.so.1`, `libz.so.2` are in the IFS for QNX
  tools; whether they can go depends on what of QNX stays. Measure with
  `readelf -d` over the final file lists.
- **Graphics** (Screen, the GPU drivers) is QNX's and stays; the ported
  graphics stack (Mesa, Wayland, GTK, Qt) sits on top of it as in the lab's
  graphics images.
- **Licences**: an image contains QNX run-time files; it is built from a
  licensed SDP and is not something this repository distributes.
