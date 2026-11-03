# Base QEMU images (x86_64, aarch64le)

A stock `mkqnximage` image from the QNX SDP is all the packages need, plus a few files copied once
(section 4). These are the images the packages are tested on.

## 1. Host setup (Ubuntu 24.04, once)

```bash
host$ SDP_DIR=~/qnx800                    # QNX SDP 8.0 install path
host$ SSH_KEY=~/.ssh/qnx-target           # SSH key for the targets
host$ source $SDP_DIR/qnxsdp-env.sh
host$ which mkqnximage
host$ sudo apt install qemu-system-x86 qemu-system-arm qemu-utils libvirt-daemon-system bridge-utils
host$ sudo usermod -aG kvm,libvirt $USER  # then log out and in again
host$ virsh net-start default; virsh net-autostart default
host$ echo "allow virbr0" | sudo tee /etc/qemu/bridge.conf
host$ sudo chmod u+s /usr/lib/qemu/qemu-bridge-helper
host$ ssh-keygen -t ed25519 -N '' -f $SSH_KEY
```

## 2. Make and boot an image

Defaults of `mkqnximage`, with only `/data` sized for packages (the default `/data` is too small):

| Purpose | `/data` options |
|---|---|
| a few packages at a time | `--data-size=4000 --data-inodes=80000` |
| **every package** (about 8 GB, 180,000 files installed) | `--data-size=16000 --data-inodes=320000` |

```bash
# x86_64 (runs under KVM)
host$ mkdir -p ~/vm/x86 && cd ~/vm/x86
host$ TARGET_NAME=qnx-x86; SDP_ARCH=x86_64
host$ mkqnximage --type=qemu --arch=x86_64 --hostname=$TARGET_NAME \
        --ssh-ident=$SSH_KEY.pub \
        --data-size=16000 --data-inodes=320000 --ram=2G --cpu=4 --run=-h
host$ TARGET_IP=$(mkqnximage --getip)

# aarch64le (another directory; emulated, so slower)
host$ mkdir -p ~/vm/a64 && cd ~/vm/a64
host$ TARGET_NAME=qnx-a64; SDP_ARCH=aarch64le
host$ mkqnximage --type=qemu --arch=aarch64le --hostname=$TARGET_NAME \
        --ssh-ident=$SSH_KEY.pub \
        --data-size=16000 --data-inodes=320000 --ram=2G --cpu=4 --run=-h
host$ TARGET_IP=$(mkqnximage --getip)
```

In the VM's directory: `mkqnximage --stop`, `mkqnximage --run=-h` (start again), `mkqnximage --wipe`
(delete). Two VMs need different host names. When you switch between them, set `TARGET_IP` and
`SDP_ARCH` again in that VM's directory.

```bash
host$ ssh -i $SSH_KEY root@$TARGET_IP 'uname -a; df -h /data'
```

## 3. Optional: an image with graphics (`--graphics=yes`)

To run graphical packages on screen too (the labwc/Xfce desktop, GTK, Qt, SDL programs, MAME,
RetroArch, VLC, ...), create the image **again** with `--graphics=yes`. Changing options alone gives
"Nothing to do", so use `--clean --build`:

```bash
host$ mkqnximage --clean --build --type=qemu --arch=x86_64 --hostname=$TARGET_NAME --graphics=yes \
        --ssh-ident=$SSH_KEY.pub \
        --data-size=16000 --data-inodes=320000 --ram=2G --cpu=4 --run=-h
```

- It needs QNX Screen's VirtIO graphics support (*Screen Board Support VirtIO*) in the SDP.
- x86_64 must run under **KVM** (`[ -w /dev/kvm ]`); without it, `screen` crashes. The QEMU window
  opens on the host's desktop.
- Check with `gles2-gears` on the target. For screenshots, copy the SDP's
  `$SDP_DIR/target/qnx/$SDP_ARCH/usr/bin/screenshot` to the target. QEMU's `screendump` cannot read the
  GL display.

## 4. What a stock image lacks (from the host, once per VM)

**① A CA bundle**, so that https to github.com can be verified:

```bash
host$ ssh -i $SSH_KEY root@$TARGET_IP 'mkdir -p /data/oss-setup /data/oss/qnx-runtime/lib'
host$ scp -i $SSH_KEY /etc/ssl/certs/ca-certificates.crt root@$TARGET_IP:/data/oss-setup/cacert.pem
```

**② QNX run-time libraries** the packages use and the image does not carry (they are in your SDP):

```bash
host$ for LIB in libeventfd.so.1 libepoll.so.1 libsignalfd.so.1 libtimerfd.so.1 libasound.so.5 \
               libscreen.so.1 libEGL.so.1 libGLESv2.so.1 libbacktrace.so.1 \
               libdrm.so.1 libdrm.so.2 libgbm.so.1 libnetmap.so.1 dri_gbm.so; do
        for SDP_LIB_DIR in $SDP_DIR/target/qnx/$SDP_ARCH/usr/lib $SDP_DIR/target/qnx/$SDP_ARCH/lib; do
          [ -f $SDP_LIB_DIR/$LIB ] && { scp -i $SSH_KEY $SDP_LIB_DIR/$LIB root@$TARGET_IP:/data/oss/qnx-runtime/lib/; break; }
        done
      done
```

A library the image already has (`ls /proc/boot /system/lib /usr/lib /lib` on the target) need not
be copied.

Shortcut: [`scripts/target-bootstrap`](https://github.com/openqnx/oss/blob/main/scripts/target-bootstrap)
`<target-ip>` from a clone of the repository does ①, ② and the apk setup of
[Install with apk](Install-with-apk) in one go.

## 5. On the target, at every login

```sh
qnx# ARCH=x86_64                     # aarch64 VM: ARCH=aarch64
qnx# GH_RELEASES=https://github.com/openqnx/oss/releases/download
qnx# mkdir -p /data/oss/usr/local
qnx# ln -sP /data/oss/usr/local /usr/local    # again after every reboot
qnx# export PATH=/usr/local/bin:/usr/local/sbin:$PATH
qnx# export LD_LIBRARY_PATH=/usr/local/lib:/data/oss/qnx-runtime/lib
qnx# export SSL_CERT_FILE=/data/oss-setup/cacert.pem
```

Next: [Install with apk](Install-with-apk) or [Install the archives](Install-the-archives).
