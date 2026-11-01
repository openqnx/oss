# e2fsprogs: the ext2/ext3/ext4 file system utilities (mke2fs, e2fsck,
# tune2fs, debugfs, dumpe2fs, resize2fs, e2label, ...) and their libraries,
# libext2fs, libe2p, libcom_err and libss.
VERSION=1.47.4
TARBALL=https://mirrors.edge.kernel.org/pub/linux/kernel/people/tytso/e2fsprogs/v1.47.4/e2fsprogs-1.47.4.tar.xz
SHA256=fd5bf388cbdbe006a3d3b318d983b2948382440acc85a87f1e7d108653e8db0b
BUILDSYS=autotools
# libuuid comes from its own port (util-linux's); e2fsprogs would otherwise
# install a private copy with the same soname and headers. Its libblkid stays
# (no other port provides one): --enable-libblkid builds the private one
# always. Left to configure, it takes any libblkid.so it finds in staging --
# an aarch64le build linked one that a passing build had left there and no
# package provides, and every program failed at load with "Could not load
# library libblkid.so.1".
DEPS="libuuid"
CONF_ARGS_EXTRA=( --enable-elf-shlibs --disable-nls --disable-fuse2fs --disable-uuidd --disable-libuuid --enable-libblkid
                  --disable-e2initrd-helper --disable-defrag --without-systemd-unit-dir
                  --without-udev-rules-dir --without-crond-dir )
# badblocks measures its progress line with mbstowcs() + wcswidth() when
# configure finds mbstowcs(); QNX has no wcswidth(). Its only locale is "C",
# where strlen() is the width, which is what badblocks falls back to.
# LDCONFIG=: -- "make install" would otherwise run the build machine's ldconfig.
CONF_ENV=( ac_cv_func_mbstowcs=no LDCONFIG=: )
