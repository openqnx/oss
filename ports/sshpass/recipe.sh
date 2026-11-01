# sshpass: runs ssh (or any program that asks for a password on its terminal)
# with the password given on the command line, a file, a descriptor or the
# environment: it starts the program on a pseudo-terminal of its own and
# answers the prompt.
VERSION=1.10
TARBALL=https://downloads.sourceforge.net/project/sshpass/sshpass/1.10/sshpass-1.10.tar.gz
SHA256=ad1106c203cbb56185ca3bad8c6ccafca3b4064696194da879f81c8d7bdfeeda
BUILDSYS=autotools
# AC_FUNC_MALLOC cannot run its test when cross-compiling and assumes
# malloc(0) returns NULL, then calls an rpl_malloc() nothing provides. QNX's
# malloc(0) returns a unique pointer (glibc behaviour).
CONF_ENV=( ac_cv_func_malloc_0_nonnull=yes )
