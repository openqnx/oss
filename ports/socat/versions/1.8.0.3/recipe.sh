# socat: relays bytes between two addresses of almost any kind (TCP, UDP,
# UNIX sockets, files, pipes, ptys, programs, OpenSSL connections).
VERSION=1.8.0.3
TARBALL=http://www.dest-unreach.org/socat/download/socat-1.8.0.3.tar.gz
SHA256=a9f9eb6cfb9aa6b1b4b8fe260edbac3f2c743f294db1e362b932eb3feca37ba4
BUILDSYS=autotools
DEPS="openssl readline"
# regcomp()/regexec() (socat's address matching) are in QNX's libregex, not
# libc.
CONF_ENV=( LIBS=-lregex )
