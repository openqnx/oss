# GNU make. Version as in the QNX Everywhere 8.0.4r1 image.
VERSION=4.4.1
TARBALL=https://ftp.gnu.org/gnu/make/make-4.4.1.tar.gz
SHA256=dd16fb1d67bfab79a72f5e8390735c49e3e8e70b4945a15ab1f81ddb78658fb3
BUILDSYS=autotools
CONF_ARGS_EXTRA=( --disable-nls --without-guile )
# make's lib/ glob and fnmatch (GNU glob_t with gl_opendir & co.) under their
# own names: the toolchain puts libc ahead of the objects to keep it NEEDED,
# so a plain 'glob' resolved to QNX libc's, whose glob_t differs -- every
# $(wildcard) crashed (SDP 8.0.6 rebuild).
CFLAGS_APPEND="-Dglob=rpl_glob -Dglobfree=rpl_globfree -Dfnmatch=rpl_fnmatch"
