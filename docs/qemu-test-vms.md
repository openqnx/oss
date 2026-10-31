# Test VMs: reproducing the QNX targets every result here was recorded on

Every component in this repository was installed and tested on two QEMU
virtual machines built from a **stock QNX SDP 8.0 image**, with nothing from
this repository inside the image. This page shows how to build the same two
machines yourself, one command at a time, with your own QNX SDP 8.0 license --
and how to install and test the packages on them by hand.

`vanilla-vm/` in this repository automates the same steps; you do not need it.
Everything below is plain `mkqnximage`, `ssh` and `apk`.

The results in `tests/results/<arch>.tsv` and the per-package logs in
`tests/logs/<arch>/` name these targets as
"QEMU x86_64, vanilla mkqnximage image" and
"QEMU aarch64le, vanilla mkqnximage image".

---

## 1. What you need

| | |
|---|---|
| A QNX SDP 8.0 license | a free non-commercial license from <https://www.qnx.com/getqnx>, or a commercial one |
| A Linux host | x86_64; tested on Ubuntu 24.04 |
| QNX SDP 8.0 | installed with the QNX Software Center (step 2) |
| QEMU | `qemu-system-x86_64`, `qemu-system-aarch64` (QEMU 8 or later; the results were recorded with 10.1) |
| KVM | for the x86_64 VM (the aarch64 VM is emulated) |
| A network bridge | `virbr0`, libvirt's default network |
| Disk | about 10 GB per VM |

## 2. Install QNX SDP 8.0

1. With your myQNX account, download the **QNX Software Center** for Linux
   (the "Get QNX" page above leads there).
2. Install and start it, sign in, and add your license key.
3. Add an installation of **QNX Software Development Platform 8.0**, e.g.
   into `~/qnx800`.
4. Only for the graphics VM (step 9): add the package **Screen Board Support
   VirtIO** to that installation (search for "VirtIO" in its package list).

Check:

```bash
source ~/qnx800/qnxsdp-env.sh
which mkqnximage qcc            # both found
```

## 3. Prepare the host: QEMU, KVM, the bridge

```bash
sudo apt install qemu-system-x86 qemu-system-arm qemu-utils libvirt-daemon-system bridge-utils
sudo usermod -aG kvm,libvirt $USER       # then log out and in again
virsh net-start default 2>/dev/null; virsh net-autostart default
ip addr show virbr0                       # 192.168.122.1/24
# QEMU's bridge helper may attach VMs to virbr0:
echo "allow virbr0" | sudo tee /etc/qemu/bridge.conf
sudo chmod u+s /usr/lib/qemu/qemu-bridge-helper
```

`mkqnximage` uses `virbr0` when it exists, and KVM when `/dev/kvm` is
writable (it warns otherwise).

An SSH key for the targets:

```bash
ssh-keygen -t ed25519 -N '' -f ~/.ssh/qnx-target
```

## 4. Build and boot the x86_64 VM

A VM is a directory; `mkqnximage` keeps its configuration (`local/`) and the
image (`output/`) in the current directory.

```bash
source ~/qnx800/qnxsdp-env.sh
mkdir -p ~/vm/oss-vanilla-x86_64 && cd ~/vm/oss-vanilla-x86_64
mkqnximage --type=qemu --arch=x86_64 --hostname=vanilla-x86 \
           --ssh-ident=$HOME/.ssh/qnx-target.pub \
           --data-size=4000 --data-inodes=80000 \
           --ram=2G --cpu=4 \
           --run=-h
```

These are exactly the options the recorded results used:

| Option | Meaning |
|---|---|
| `--type=qemu --arch=x86_64` | a QEMU image for x86_64 |
| `--hostname=vanilla-x86` | the target's name |
| `--ssh-ident=...pub` | your public key, so `ssh root@<ip>` works without a password |
| `--data-size=4000 --data-inodes=80000` | a 4 GB `/data` partition: packages are installed there (`/data/oss`). 4 GB is enough to test packages one at a time; to install **all** of them at once use `--data-size=16000 --data-inodes=320000` |
| `--ram=2G --cpu=4` | memory and CPUs (use `--ram=4G` for PyTorch, ROS 2, whisper.cpp at once) |
| `--run=-h` | build if needed, then start QEMU in the background (headless) |

Everything else is `mkqnximage`'s default -- in particular **no graphics**
(`--graphics=no`) and none of this repository's files.

Wait for it to boot (half a minute with KVM), then:

```bash
mkqnximage --getip                     # e.g. 192.168.122.<n>
ssh -i ~/.ssh/qnx-target root@<ip> 'uname -a; df -h /data'
```

Stop and start again (from the VM's directory):

```bash
mkqnximage --stop
mkqnximage --run=-h                    # starts again without rebuilding
mkqnximage --wipe                      # deletes the VM
```

## 5. Build and boot the aarch64 VM

The same, with `--arch=aarch64le` in its own directory:

```bash
mkdir -p ~/vm/oss-vanilla-aarch64le && cd ~/vm/oss-vanilla-aarch64le
mkqnximage --type=qemu --arch=aarch64le --hostname=vanilla-a64 \
           --ssh-ident=$HOME/.ssh/qnx-target.pub \
           --data-size=4000 --data-inodes=80000 \
           --ram=2G --cpu=4 \
           --run=-h
mkqnximage --getip
```

QEMU emulates Arm on an x86 host (`virt-4.2` machine, Cortex-A57): it is
correct, and several times slower -- the first boot takes a few minutes.

## 6. What a stock image lacks, and how to add it

Two things, both copied from your host:

**A CA certificate bundle.** A stock image has none, so `curl` cannot verify
`https://github.com`. Copy your host's:

```bash
T=<target ip>
ssh -i ~/.ssh/qnx-target root@$T 'mkdir -p /data/oss-setup'
scp -i ~/.ssh/qnx-target /etc/ssl/certs/ca-certificates.crt root@$T:/data/oss-setup/cacert.pem
```

**Some QNX run-time libraries** the packages use and the stock image does not
carry. They are QNX's own libraries, part of your SDP:

```bash
ARCH=x86_64                         # or aarch64le
ssh -i ~/.ssh/qnx-target root@$T 'mkdir -p /data/oss/qnx-runtime/lib'
for l in libeventfd.so.1 libepoll.so.1 libsignalfd.so.1 libtimerfd.so.1 libasound.so.5 \
         libscreen.so.1 libEGL.so.1 libGLESv2.so.1 libbacktrace.so.1 \
         libdrm.so.1 libdrm.so.2 libgbm.so.1; do
  for d in ~/qnx800/target/qnx/$ARCH/usr/lib ~/qnx800/target/qnx/$ARCH/lib; do
    [ -f $d/$l ] && { scp -i ~/.ssh/qnx-target $d/$l root@$T:/data/oss/qnx-runtime/lib/; break; }
  done
done
```

(A library the image already has -- `ls /proc/boot /system/lib /usr/lib /lib`
on the target -- need not be copied.)

## 7. Set up apk on the target

Copy the setup script and the signing key from this repository, and run it:

```bash
cd <this repository>
scp -i ~/.ssh/qnx-target target/apk-setup target/oss-get keys/openqnx-oss-20261029.rsa.pub root@$T:/data/oss-setup/
ssh -i ~/.ssh/qnx-target root@$T
```

On the target:

```sh
cd /data/oss-setup
sh ./apk-setup -c /data/oss-setup/cacert.pem -k /data/oss-setup/openqnx-oss-20261029.rsa.pub
. /data/oss/etc/oss-env.sh
apk update
```

`apk-setup` explains in its header what it does; in short: it fetches
`apk-tools` (and `openssl zlib zstd ca-certificates`) from the
`packages-8.0-<arch>` release, points apk at the `apk-8.0-<arch>` and
`tests-8.0-<arch>` releases with the signing key, lets apk install itself into
`/data/oss`, and links `/usr/local` to `/data/oss/usr/local`.

After every reboot of the target: `ln -sP /data/oss/usr/local /usr/local`.

## 8. Install a package and run its test -- as the results were recorded

```sh
. /data/oss/etc/oss-env.sh
apk add zstd qnx-test-zstd qnx-port-tests
QNX_PORT_TESTS_VERBOSE=1 /usr/local/bin/qnx-port-tests/run-on-target.sh test_zstd
```

Every check prints `ok` or `FAIL`, a test ends with `PASS`/`FAIL <test>`.
This is the command in each package's log, `tests/logs/<arch>/<name>-<version>.log`,
so you can compare your output with the recorded one line by line.
Which test belongs to which component: `tests/port-tests.tsv`. A component
without a test of its own was checked by loading every program and library it
installs (`ldd`), as its log shows.

Remove it again: `apk del zstd qnx-test-zstd`.

## 9. The graphics VM (QNX Screen, the desktop)

With the **Screen Board Support VirtIO** package installed (step 2), add
`--graphics=yes` (in a directory of its own). QEMU then opens a window
(virtio GPU) and the image starts QNX Screen at boot:

```bash
mkdir -p ~/vm/oss-desktop-x86_64 && cd ~/vm/oss-desktop-x86_64
mkqnximage --type=qemu --arch=x86_64 --hostname=desktop-x86 \
           --ssh-ident=$HOME/.ssh/qnx-target.pub \
           --data-size=8000 --data-inodes=160000 --ram=4G --cpu=4 \
           --graphics=yes --run=-h
```

Then steps 6-7, and on the target `apk add oss-desktop && oss-desktop`.
