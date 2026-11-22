# socat: relays bytes between two addresses of almost any kind (TCP, UDP,
# UNIX sockets, files, pipes, ptys, programs, OpenSSL connections).
VERSION=1.8.1.3
TARBALL=http://www.dest-unreach.org/socat/download/socat-1.8.1.3.tar.gz
SHA256=06602ffd591e98c75b3dc1d66f0f19136cc666b0b2d95caad987d6ab2cb28097
BUILDSYS=autotools
DEPS="openssl readline"
# regcomp()/regexec() (socat's address matching) are in QNX's libregex, not
# libc.
CONF_ENV=( LIBS=-lregex )
