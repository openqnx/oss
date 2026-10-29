VERSION=4.33; TARBALL=http://dist.schmorp.de/libev/libev-4.33.tar.gz
SHA256=507eb7b8d1015fbec5b935f34ebed15bf346bed04a11ab82b8eee848c4205aea
BUILDSYS=autotools
# Once the driver started linking QNX's split libc (libepoll, libeventfd,
# libsignalfd, libfsnotify) into every configure probe, libev's configure began
# finding epoll_ctl, eventfd, signalfd and inotify_init, where the earlier build
# had found none and used poll().
#
# epoll/eventfd/signalfd stay on: ev_epoll.c uses level-triggered epoll only
# (no EPOLLET/EPOLLONESHOT), which is what QNX 8.0 provides.
# inotify is turned off: ev.c's inotify path needs <sys/statfs.h> and statfs()
# to tell local from remote filesystems, and QNX has only statvfs.  ev_stat
# watchers fall back to periodic stat(), which is libev's portable path.
CONF_ENV=( ac_cv_func_inotify_init=no )
CONF_ARGS_EXTRA=( --enable-shared --disable-static )
# libev's event.h is its emulation of the libevent API, under the same name as
# libevent's real header (the libevent component). Do not install it; ev.h is
# libev's own API.
do_install() { make -C "$B" DESTDIR="$STAGE" install && rm -f "$STAGE$PREFIX/include/event.h"; }
