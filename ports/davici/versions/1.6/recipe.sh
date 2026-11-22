# davici: decoupled asynchronous client library for strongSwan's VICI
# protocol (the charon daemon's control interface), LGPL, without
# libstrongswan or threads: the caller's poll() loop drives it.
VERSION=1.6; GIT=https://github.com/strongswan/davici.git; REF=v1.6
BUILDSYS=autotools
RUNTIME_DEPS="strongswan"
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
