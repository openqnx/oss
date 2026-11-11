/*
 * libqsocketcan -- Linux SocketCAN sockets (PF_CAN, SOCK_RAW, CAN_RAW) for
 * programs on QNX 8.0, whose network stack has no PF_CAN.
 *
 * A program built with qnx-socketcan's <linux/can.h> and linked with this
 * library (before libsocket: `pkg-config --libs qnx-socketcan`) calls
 * socket(PF_CAN, SOCK_RAW, CAN_RAW), bind(), setsockopt(), read(), write(),
 * recvmsg(), ioctl(SIOCGIFINDEX), ... as on Linux. The library defines
 * those functions: for a CAN socket it does the work, for every other
 * descriptor it calls the next definition (libsocket's, libc's).
 *
 * A CAN socket is a connection (AF_UNIX, SOCK_SEQPACKET) to the CAN bus
 * daemon qcand, which carries frames between the sockets of an interface and
 * applies the filters. The descriptor is a real socket: select(), poll(),
 * fcntl(O_NONBLOCK), close() and fork() work on it as they are, and so do
 * read() and write(): a message on the connection is a CAN frame as it is.
 *
 * Link order. libsocket's functions (socket, bind, setsockopt, recvmsg,
 * sendto, ...) must be found here first, so this library goes before
 * -lsocket. libc's (read, write, close, ioctl) need not be: many builds
 * name -lc first (this workspace's CMake toolchain does), and then
 *  - read()/write() are libc's and carry frames as they are (what is lost:
 *    write()'s EINVAL/ENXIO for a frame the interface cannot take -- qcand
 *    drops it; read() returns 0, not ENETDOWN, once qcand is gone; read()
 *    on a socket that also uses recvmsg()/recvfrom() returns the frame with
 *    qcand's header after it when the buffer is larger than the frame);
 *  - recvmsg()/recvfrom() learn a frame's interface, time and "own" flag
 *    from qcand for the frames that arrive after the socket's first such
 *    call (with the library's read() in place: for every frame);
 *  - close() is libc's: the library checks on every call that a descriptor
 *    it knows as a CAN socket still is the connection it made;
 *  - ioctl() is redirected by <linux/socket.h> (#define ioctl qcan_ioctl;
 *    QCAN_NO_IOCTL_REDIRECT turns that off).
 *
 * Not SocketCAN's: CAN_BCM, CAN_ISOTP, CAN_J1939 sockets (EPROTONOSUPPORT),
 * CAN XL frames, hardware timestamps; dup()ed descriptors are not CAN
 * sockets for this library, and exec() forgets them.
 */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <linux/can.h>      /* qnx-socketcan's: pulls in <linux/socket.h> (AF_CAN, SIOCGSTAMP, ...) */
#include <linux/can/raw.h>
#include "qcan_proto.h"

#define QFD_MAX 4096
#define MAXIOV  16

struct qs {
	volatile unsigned char can;    /* this descriptor is a CAN socket */
	unsigned char ts_on, ovfl_on, bound, meta;
	uint32_t ifindex, mtu;         /* bound interface (0: all) and its MTU */
	int loopback, recv_own, fd_frames, join, xl, err_mask;
	int mark, prio;
	struct timeval last;           /* SIOCGSTAMP */
	char peer[sizeof(((struct sockaddr_un *)0)->sun_path)];   /* getpeername() of the connection */
};
static struct qs tab[QFD_MAX];

/* The next definitions in the search order: libsocket's and libc's. When
 * libc is ahead of this library there is no next read/write/close/ioctl:
 * then the first one (libc's; this library's are not reached at all). */
#define NEXT(ret, name, ...) \
	static ret (*real_##name)(__VA_ARGS__); \
	if (!real_##name) real_##name = (ret (*)(__VA_ARGS__))dlsym(RTLD_NEXT, #name); \
	if (!real_##name) real_##name = (ret (*)(__VA_ARGS__))dlsym(RTLD_DEFAULT, #name)

static int q_close(int fd);
static void want_meta(int fd);
static ssize_t q_read(int fd, void *buf, size_t len);
static int peername(int fd, char *buf, size_t n)
{
	NEXT(int, getsockopt, int, int, int, void *, socklen_t *);   /* not this library's: it asks iscan() */
	struct sockaddr_un sa; socklen_t l = sizeof sa; int t = 0; socklen_t tl = sizeof t;
	memset(&sa, 0, sizeof sa);
	if (real_getsockopt(fd, SOL_SOCKET, SO_TYPE, &t, &tl) < 0 || t != SOCK_SEQPACKET) return -1;
	if (getpeername(fd, (struct sockaddr *)&sa, &l) < 0) return errno == ENOTCONN ? 1 : -1;   /* 1: qcand is gone */
	if (!sa.sun_path[0]) return -1;
	strlcpy(buf, sa.sun_path, n);
	return 0;
}

/* Does the program's <name>() come from this library? (Compared by object:
 * a function's address taken here is the program's first definition.) */
static int ours(const char *name)
{
	Dl_info me, it; void *p = dlsym(RTLD_DEFAULT, name);
	return p && dladdr((void *)(uintptr_t)peername, &me) && dladdr(p, &it) && me.dli_fbase == it.dli_fbase;
}

/* Is fd a CAN socket? When this library's close() is the program's close(),
 * the table is right. When libc's is (see "Link order" above), a closed
 * CAN socket's number may be another file by now: ask the descriptor. */
static int iscan(int fd)
{
	static int close_is_ours = -1;
	if ((unsigned)fd >= QFD_MAX || !tab[fd].can) return 0;
	if (close_is_ours < 0) close_is_ours = ours("close");
	if (close_is_ours) return 1;
	int e = errno; char p[sizeof tab[fd].peer];
	int r = peername(fd, p, sizeof p);
	if (r > 0 || (r == 0 && !strcmp(p, tab[fd].peer))) { errno = e; return 1; }
	tab[fd].can = 0; errno = e;
	return 0;
}
#define ISCAN(fd) iscan(fd)

static const char *sockpath(void)
{
	const char *p = getenv(QCAN_SOCK_ENV);
	return p && *p ? p : QCAN_SOCK_PATH;
}

static int qconnect(int flags)
{
	NEXT(int, socket, int, int, int);
	NEXT(int, close, int);
	struct sockaddr_un sa = { .sun_family = AF_UNIX };
	strlcpy(sa.sun_path, sockpath(), sizeof sa.sun_path);
	int fd = real_socket(AF_UNIX, SOCK_SEQPACKET | flags, 0);
	if (fd < 0) return -1;
	/* A non-blocking connect to a local listener completes at once or fails. */
	if (connect(fd, (struct sockaddr *)&sa, sizeof sa) < 0) { real_close(fd); errno = ENETDOWN; return -1; }
	return fd;
}

/* One request to qcand on a connection of its own; the reply's header in *rh,
 * its payload (a name) in name[QCAN_IFNAMSIZ]. 0, or -1 with errno. */
static int ctl(uint32_t type, uint32_t ifindex, const char *pname, struct qcan_hdr *rh, char *name)
{
	NEXT(int, close, int);
	NEXT(ssize_t, sendmsg, int, const struct msghdr *, int);
	NEXT(ssize_t, recvmsg, int, struct msghdr *, int);
	int fd = qconnect(SOCK_CLOEXEC), e;
	if (fd < 0) return -1;
	struct qcan_hdr h = { .type = type, .ifindex = ifindex };
	char nb[QCAN_IFNAMSIZ] = "";
	if (pname) strlcpy(nb, pname, sizeof nb);
	struct iovec iov[2] = { { &h, sizeof h }, { nb, pname ? strlen(nb) + 1 : 0 } };
	struct msghdr m = { .msg_iov = iov, .msg_iovlen = pname ? 2 : 1 };
	if (pname && strlen(pname) >= QCAN_IFNAMSIZ) { real_close(fd); errno = ENODEV; return -1; }
	if (real_sendmsg(fd, &m, 0) < 0) { e = errno; real_close(fd); errno = e; return -1; }
	iov[0].iov_base = rh; iov[1].iov_base = name; iov[1].iov_len = QCAN_IFNAMSIZ; m.msg_iovlen = 2;
	ssize_t n = real_recvmsg(fd, &m, 0);
	e = errno; real_close(fd);
	if (n < (ssize_t)sizeof *rh) { errno = n < 0 ? e : ENETDOWN; return -1; }
	name[QCAN_IFNAMSIZ - 1] = 0;
	if (rh->type != QCAN_IFINFO) { errno = rh->arg ? (int)rh->arg : ENODEV; return -1; }
	return 0;
}

static int qmsg(int fd, uint32_t type, uint32_t ifindex, uint32_t arg, const void *p, size_t n)
{
	NEXT(ssize_t, sendmsg, int, const struct msghdr *, int);
	struct qcan_hdr h = { .type = type, .ifindex = ifindex, .arg = arg };
	struct iovec iov[2] = { { &h, sizeof h }, { (void *)p, n } };
	struct msghdr m = { .msg_iov = iov, .msg_iovlen = n ? 2 : 1 };
	return real_sendmsg(fd, &m, 0) < 0 ? -1 : 0;
}

/* ---- socket, bind, close -------------------------------------------------- */

int socket(int domain, int type, int protocol)
{
	NEXT(int, socket, int, int, int);
	NEXT(int, close, int);
	if (domain != AF_CAN) return real_socket(domain, type, protocol);
	int flags = type & (SOCK_CLOEXEC | SOCK_NONBLOCK);
	if ((type & ~flags) != SOCK_RAW || protocol != CAN_RAW) { errno = EPROTONOSUPPORT; return -1; }
	/* Connect blocking, then set O_NONBLOCK: the socket's messages to qcand are synchronous. */
	int fd = qconnect(flags & SOCK_CLOEXEC);
	if (fd < 0) return -1;
	if (fd >= QFD_MAX) { real_close(fd); errno = EMFILE; return -1; }
	memset(&tab[fd], 0, sizeof tab[fd]);
	tab[fd].loopback = 1;
	peername(fd, tab[fd].peer, sizeof tab[fd].peer);
	if (flags & SOCK_NONBLOCK) { NEXT(int, ioctl, int, int, ...); int on = 1; real_ioctl(fd, FIONBIO, &on); }
	tab[fd].can = 1;
	/* When the program's read() is this library's, every frame can come with
	 * what qcand knows about it from the start (see want_meta()). */
	static int read_is_ours = -1;
	if (read_is_ours < 0) read_is_ours = ours("read");
	if (read_is_ours) want_meta(fd);
	return fd;
}

int bind(int fd, const struct sockaddr *addr, socklen_t len)
{
	NEXT(int, bind, int, const struct sockaddr *, socklen_t);
	if (!ISCAN(fd)) return real_bind(fd, addr, len);
	const struct sockaddr_can *ca = (const struct sockaddr_can *)addr;
	if (!addr || len < (socklen_t)(offsetof(struct sockaddr_can, can_ifindex) + sizeof ca->can_ifindex) || ca->can_family != AF_CAN) { errno = EINVAL; return -1; }
	uint32_t mtu = CANFD_MTU;
	if (ca->can_ifindex) {
		struct qcan_hdr rh; char name[QCAN_IFNAMSIZ];
		if (ctl(QCAN_IFBYINDEX, (uint32_t)ca->can_ifindex, NULL, &rh, name) < 0) { if (errno != ENETDOWN) errno = ENODEV; return -1; }
		mtu = rh.arg;
	}
	if (qmsg(fd, QCAN_BIND, (uint32_t)ca->can_ifindex, 0, NULL, 0) < 0) return -1;
	tab[fd].ifindex = (uint32_t)ca->can_ifindex; tab[fd].mtu = mtu; tab[fd].bound = 1;
	return 0;
}

static int q_close(int fd)
{
	NEXT(int, close, int);
	if ((unsigned)fd < QFD_MAX) tab[fd].can = 0;
	return real_close(fd);
}
int close(int fd) { return q_close(fd); }

/* ---- options -------------------------------------------------------------- */

static int *rawopt(struct qs *s, int name)
{
	switch (name) {
	case CAN_RAW_LOOPBACK:      return &s->loopback;
	case CAN_RAW_RECV_OWN_MSGS: return &s->recv_own;
	case CAN_RAW_FD_FRAMES:     return &s->fd_frames;
	case CAN_RAW_JOIN_FILTERS:  return &s->join;
	case CAN_RAW_ERR_FILTER:    return &s->err_mask;
	case CAN_RAW_XL_FRAMES:     return &s->xl;
	}
	return NULL;
}

int setsockopt(int fd, int level, int name, const void *val, socklen_t len)
{
	NEXT(int, setsockopt, int, int, int, const void *, socklen_t);
	if (!ISCAN(fd)) return real_setsockopt(fd, level, name, val, len);
	struct qs *s = &tab[fd];
	int v = 0;
	if (val && len >= sizeof v) memcpy(&v, val, sizeof v);
	if (level == SOL_CAN_RAW) {
		if (name == CAN_RAW_FILTER) {
			if (len % sizeof(struct can_filter) || len > CAN_RAW_FILTER_MAX * sizeof(struct can_filter)) { errno = EINVAL; return -1; }
			/* The filters and 4 bytes more: "no filter" is a payload too, and no
			 * message is as long as a CAN FD frame (qcan_proto.h). */
			char fb[CAN_RAW_FILTER_MAX * sizeof(struct can_filter) + 4];
			memset(fb, 0, sizeof fb);
			if (len) memcpy(fb, val, len);
			return qmsg(fd, QCAN_FILTER, 0, 0, fb, len + 4);
		}
		if (name == CAN_RAW_XL_VCID_OPTS) return 0;
		int *o = rawopt(s, name);
		if (!o) { errno = ENOPROTOOPT; return -1; }
		if (len < sizeof v) { errno = EINVAL; return -1; }
		/* XL is accepted (candump always asks for it) and no XL frame ever arrives. */
		if (name != CAN_RAW_XL_FRAMES && qmsg(fd, QCAN_OPT, 0, (uint32_t)name, &v, sizeof v) < 0) return -1;
		*o = name == CAN_RAW_ERR_FILTER ? v : v != 0;
		return 0;
	}
	if (level == SOL_SOCKET)
		switch (name) {
		case SO_TIMESTAMP:   s->ts_on = v != 0; return 0;
		case SO_RXQ_OVFL:    s->ovfl_on = v != 0; return 0;
		case SO_MARK:        s->mark = v; return 0;
		case SO_PRIORITY:    s->prio = v; return 0;
		case SO_RCVBUFFORCE: errno = EPERM; return -1;         /* callers fall back to SO_RCVBUF */
		case SO_TIMESTAMPING:
		case SO_TXTIME:      errno = ENOPROTOOPT; return -1;    /* no hardware timestamps, no timed sending */
		}
	return real_setsockopt(fd, level, name, val, len);
}

int getsockopt(int fd, int level, int name, void *val, socklen_t *len)
{
	NEXT(int, getsockopt, int, int, int, void *, socklen_t *);
	if (!ISCAN(fd)) return real_getsockopt(fd, level, name, val, len);
	struct qs *s = &tab[fd];
	int *o = NULL, ts;
	if (level == SOL_CAN_RAW) {
		o = rawopt(s, name);
		if (!o) { errno = name == CAN_RAW_FILTER ? EOPNOTSUPP : ENOPROTOOPT; return -1; }
	} else if (level == SOL_SOCKET) {
		if (name == SO_MARK) o = &s->mark;
		else if (name == SO_PRIORITY) o = &s->prio;
		else if (name == SO_TIMESTAMP) { ts = s->ts_on; o = &ts; }
		else if (name == SO_RXQ_OVFL) { ts = s->ovfl_on; o = &ts; }
	}
	if (!o) return real_getsockopt(fd, level, name, val, len);
	if (!val || !len || *len < sizeof(int)) { errno = EINVAL; return -1; }
	memcpy(val, o, sizeof(int)); *len = sizeof(int);
	return 0;
}

/* ---- interfaces ----------------------------------------------------------- */

unsigned int if_nametoindex(const char *name)
{
	NEXT(unsigned int, if_nametoindex, const char *);
	struct qcan_hdr rh; char nb[QCAN_IFNAMSIZ];
	int e = errno;
	if (name && ctl(QCAN_IFBYNAME, 0, name, &rh, nb) == 0) return rh.ifindex;
	errno = e;
	return real_if_nametoindex(name);
}

char *if_indextoname(unsigned int idx, char *buf)
{
	NEXT(char *, if_indextoname, unsigned int, char *);
	struct qcan_hdr rh; char nb[QCAN_IFNAMSIZ];
	int e = errno;
	if (idx >= QCAN_IFBASE && buf && ctl(QCAN_IFBYINDEX, idx, NULL, &rh, nb) == 0) { strlcpy(buf, nb, IFNAMSIZ); return buf; }
	errno = e;
	return real_if_indextoname(idx, buf);
}

static int q_ioctl(int fd, int cmd, void *arg)
{
	NEXT(int, ioctl, int, int, ...);
	if (!ISCAN(fd)) return real_ioctl(fd, cmd, arg);
	struct ifreq *ifr = arg;
	struct qcan_hdr rh; char nb[QCAN_IFNAMSIZ];
	switch ((unsigned)cmd) {
	case (unsigned)SIOCGSTAMP:
		/* No frame seen by the library: none read yet (ENOENT on Linux), or
		 * read by libc's read() -- then the time is now, near enough. */
		if (!tab[fd].last.tv_sec) {
			if (dlsym(RTLD_DEFAULT, "read") == (void *)q_read) { errno = ENOENT; return -1; }
			gettimeofday((struct timeval *)arg, NULL);
			return 0;
		}
		*(struct timeval *)arg = tab[fd].last;
		return 0;
	case (unsigned)SIOCGIFNAME:
		if (ctl(QCAN_IFBYINDEX, (uint32_t)ifr->ifr_ifindex, NULL, &rh, nb) < 0) { errno = ENODEV; return -1; }
		strlcpy(ifr->ifr_name, nb, IFNAMSIZ);
		return 0;
	case (unsigned)SIOCGIFINDEX:
	case (unsigned)SIOCGIFMTU:
	case (unsigned)SIOCGIFFLAGS:
		ifr->ifr_name[IFNAMSIZ - 1] = 0;
		if (ctl(QCAN_IFBYNAME, 0, ifr->ifr_name, &rh, nb) < 0) { errno = ENODEV; return -1; }
		if ((unsigned)cmd == (unsigned)SIOCGIFINDEX) ifr->ifr_ifindex = (int)rh.ifindex;
		else if ((unsigned)cmd == (unsigned)SIOCGIFMTU) ifr->ifr_mtu = (int)rh.arg;
		else ifr->ifr_flags = IFF_UP | IFF_RUNNING;
		return 0;
	}
	return real_ioctl(fd, cmd, arg);
}
int ioctl(int fd, int cmd, ...)
{
	va_list ap; va_start(ap, cmd);
	void *arg = va_arg(ap, void *);
	va_end(ap);
	return q_ioctl(fd, cmd, arg);
}
/* What <linux/socket.h> makes of ioctl(): reached whatever the link order. */
int qcan_ioctl(int fd, int cmd, ...)
{
	va_list ap; va_start(ap, cmd);
	void *arg = va_arg(ap, void *);
	va_end(ap);
	return q_ioctl(fd, cmd, arg);
}

/* ---- receiving ------------------------------------------------------------ */

/* The next frame for fd into the caller's buffers; what is known about it
 * in *h (from qcand in meta mode, else from the socket). Returns the length
 * copied, or -2 when fd turns out not to be a socket (closed behind the
 * library's back and reused): the caller then takes the ordinary path. */
static ssize_t qrecv(int fd, const struct iovec *uiov, int n, int flags, struct qcan_hdr *h, int *trunc)
{
	NEXT(ssize_t, recvmsg, int, struct msghdr *, int);
	char b[CANFD_MTU + sizeof *h];
	if (n < 0 || n > MAXIOV) { errno = EMSGSIZE; return -1; }
	for (;;) {
		struct iovec iov = { b, sizeof b };
		struct msghdr m = { .msg_iov = &iov, .msg_iovlen = 1 };
		ssize_t r = real_recvmsg(fd, &m, flags);
		if (r < 0) {
			if (errno == ENOTSOCK) { tab[fd].can = 0; return -2; }
			return -1;
		}
		if (r == 0) { errno = ENETDOWN; return -1; }   /* qcand is gone */
		size_t flen;
		if (r == CAN_MTU || r == CANFD_MTU) {
			struct timeval now; gettimeofday(&now, NULL);
			flen = r; memset(h, 0, sizeof *h);
			h->type = QCAN_FRAME; h->ifindex = tab[fd].ifindex;
			h->ts_ns = (uint64_t)now.tv_sec * 1000000000ull + (uint64_t)now.tv_usec * 1000;
		} else if (r == (ssize_t)(CAN_MTU + sizeof *h) || r == (ssize_t)(CANFD_MTU + sizeof *h)) {
			flen = r - sizeof *h; memcpy(h, b + flen, sizeof *h);
		} else {
			/* Not a frame (an answer nobody waits for): skip it. */
			if (flags & MSG_PEEK) { m.msg_iov = &iov; iov.iov_len = sizeof b; real_recvmsg(fd, &m, flags & ~MSG_PEEK); }
			continue;
		}
		size_t off = 0;
		for (int i = 0; i < n && off < flen; i++) {
			size_t k = uiov[i].iov_len < flen - off ? uiov[i].iov_len : flen - off;
			memcpy(uiov[i].iov_base, b + off, k); off += k;
		}
		if (trunc) *trunc = off < flen;
		if (!(flags & MSG_PEEK)) { tab[fd].last.tv_sec = h->ts_ns / 1000000000ull; tab[fd].last.tv_usec = h->ts_ns % 1000000000ull / 1000; }
		return (ssize_t)off;
	}
}

/* recvmsg() and recvfrom() tell where a frame came from, when, and whether it
 * is the socket's own: qcand sends that along from now on. Frames already
 * queued come without (the bound interface, the time of reading). */
static void want_meta(int fd)
{
	int on = 1;
	if (!tab[fd].meta && qmsg(fd, QCAN_OPT, 0, QCAN_OPT_META, &on, sizeof on) == 0) tab[fd].meta = 1;
}

static void fill_addr(struct sockaddr *addr, socklen_t *alen, uint32_t ifindex)
{
	if (!addr || !alen) return;
	struct sockaddr_can ca;
	memset(&ca, 0, sizeof ca);
	ca.can_family = AF_CAN; ca.can_ifindex = (int)ifindex;
	memcpy(addr, &ca, *alen < sizeof ca ? *alen : sizeof ca);
	*alen = sizeof ca;
}

static ssize_t q_read(int fd, void *buf, size_t len)
{
	NEXT(ssize_t, read, int, void *, size_t);
	if (ISCAN(fd)) {
		struct qcan_hdr h; struct iovec iov = { buf, len };
		ssize_t r = qrecv(fd, &iov, 1, 0, &h, NULL);
		if (r != -2) return r;
	}
	return real_read(fd, buf, len);
}
ssize_t read(int fd, void *buf, size_t len) { return q_read(fd, buf, len); }

ssize_t recv(int fd, void *buf, size_t len, int flags)
{
	NEXT(ssize_t, recv, int, void *, size_t, int);
	if (ISCAN(fd)) {
		struct qcan_hdr h; struct iovec iov = { buf, len };
		ssize_t r = qrecv(fd, &iov, 1, flags, &h, NULL);
		if (r != -2) return r;
	}
	return real_recv(fd, buf, len, flags);
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *addr, socklen_t *alen)
{
	NEXT(ssize_t, recvfrom, int, void *, size_t, int, struct sockaddr *, socklen_t *);
	if (ISCAN(fd)) {
		struct qcan_hdr h; struct iovec iov = { buf, len };
		want_meta(fd);
		ssize_t r = qrecv(fd, &iov, 1, flags, &h, NULL);
		if (r >= 0) fill_addr(addr, alen, h.ifindex);
		if (r != -2) return r;
	}
	return real_recvfrom(fd, buf, len, flags, addr, alen);
}

ssize_t recvmsg(int fd, struct msghdr *msg, int flags)
{
	NEXT(ssize_t, recvmsg, int, struct msghdr *, int);
	if (!ISCAN(fd)) return real_recvmsg(fd, msg, flags);
	struct qcan_hdr h; int trunc = 0;
	want_meta(fd);
	ssize_t r = qrecv(fd, msg->msg_iov, msg->msg_iovlen, flags, &h, &trunc);
	if (r == -2) return real_recvmsg(fd, msg, flags);
	if (r < 0) return r;
	struct qs *s = &tab[fd];
	if (msg->msg_name) { socklen_t l = msg->msg_namelen; fill_addr(msg->msg_name, &l, h.ifindex); msg->msg_namelen = l; }
	/* Every frame comes from a socket of this machine (MSG_DONTROUTE, as on a
	 * Linux vcan); MSG_CONFIRM marks the socket's own frames. */
	msg->msg_flags = MSG_DONTROUTE | (h.arg & QCAN_F_OWN ? MSG_CONFIRM : 0) | (trunc ? MSG_TRUNC : 0);
	size_t room = msg->msg_control ? msg->msg_controllen : 0, used = 0;
	if (s->ts_on && room - used >= CMSG_SPACE(sizeof(struct timeval))) {
		struct cmsghdr *c = (struct cmsghdr *)((char *)msg->msg_control + used);
		struct timeval tv = { .tv_sec = h.ts_ns / 1000000000ull, .tv_usec = h.ts_ns % 1000000000ull / 1000 };
		memset(c, 0, CMSG_SPACE(sizeof tv));
		c->cmsg_level = SOL_SOCKET; c->cmsg_type = SCM_TIMESTAMP; c->cmsg_len = CMSG_LEN(sizeof tv);
		memcpy(CMSG_DATA(c), &tv, sizeof tv);
		used += CMSG_SPACE(sizeof tv);
	}
	if (s->ovfl_on && room - used >= CMSG_SPACE(sizeof(uint32_t))) {
		struct cmsghdr *c = (struct cmsghdr *)((char *)msg->msg_control + used);
		memset(c, 0, CMSG_SPACE(sizeof(uint32_t)));
		c->cmsg_level = SOL_SOCKET; c->cmsg_type = SO_RXQ_OVFL; c->cmsg_len = CMSG_LEN(sizeof(uint32_t));
		memcpy(CMSG_DATA(c), &h.dropcnt, sizeof(uint32_t));
		used += CMSG_SPACE(sizeof(uint32_t));
	}
	/* Linux has SCM_TIMESTAMP == SO_TIMESTAMP and programs compare with either
	 * (candump: SO_TIMESTAMP); on QNX they differ, so the stamp is there under
	 * both names when the buffer has the room. */
	if (s->ts_on && SO_TIMESTAMP != SCM_TIMESTAMP && used && room - used >= CMSG_SPACE(sizeof(struct timeval))) {
		struct cmsghdr *c = (struct cmsghdr *)((char *)msg->msg_control + used);
		struct timeval tv = { .tv_sec = h.ts_ns / 1000000000ull, .tv_usec = h.ts_ns % 1000000000ull / 1000 };
		memset(c, 0, CMSG_SPACE(sizeof tv));
		c->cmsg_level = SOL_SOCKET; c->cmsg_type = SO_TIMESTAMP; c->cmsg_len = CMSG_LEN(sizeof tv);
		memcpy(CMSG_DATA(c), &tv, sizeof tv);
		used += CMSG_SPACE(sizeof tv);
	}
	msg->msg_controllen = used;
	return r;
}

/* ---- sending -------------------------------------------------------------- */

static ssize_t qsend(int fd, const struct iovec *uiov, int n, int flags, const struct sockaddr *addr, socklen_t alen)
{
	NEXT(ssize_t, sendmsg, int, const struct msghdr *, int);
	struct qs *s = &tab[fd];
	struct iovec iov[MAXIOV + 1];
	size_t len = 0;
	if (n < 0 || n > MAXIOV) { errno = EMSGSIZE; return -1; }
	for (int i = 0; i < n; i++) len += uiov[i].iov_len;
	uint32_t ifindex = 0, mtu = s->mtu;
	if (addr) {
		const struct sockaddr_can *ca = (const struct sockaddr_can *)addr;
		if (alen < (socklen_t)(offsetof(struct sockaddr_can, can_ifindex) + sizeof ca->can_ifindex) || ca->can_family != AF_CAN) { errno = EINVAL; return -1; }
		ifindex = (uint32_t)ca->can_ifindex; mtu = CANFD_MTU;
	}
	if (!ifindex && !(s->bound && s->ifindex)) { errno = ENXIO; return -1; }       /* no interface to send on */
	/* As Linux: a classic frame, or a CAN FD frame when the socket enabled them and the interface carries them. */
	if (len != CAN_MTU && !(len == CANFD_MTU && s->fd_frames && mtu >= CANFD_MTU)) { errno = EINVAL; return -1; }
	/* On the bound interface: the frame as it is. With an address: a QCAN_FRAME. */
	struct qcan_hdr h = { .type = QCAN_FRAME, .ifindex = ifindex };
	size_t hl = ifindex && !(s->bound && s->ifindex == ifindex) ? sizeof h : 0;
	iov[0].iov_base = &h; iov[0].iov_len = hl;
	memcpy(iov + 1, uiov, n * sizeof *iov);
	struct msghdr m = { .msg_iov = iov, .msg_iovlen = n + 1 };
	ssize_t r = real_sendmsg(fd, &m, flags & ~MSG_DONTROUTE);
	if (r < 0) {
		if (errno == ENOTSOCK) { s->can = 0; return -2; }
		if (errno == EPIPE || errno == ECONNRESET || errno == ENOTCONN) errno = ENETDOWN;
		return -1;
	}
	return r - (ssize_t)hl;
}

ssize_t write(int fd, const void *buf, size_t len)
{
	NEXT(ssize_t, write, int, const void *, size_t);
	if (ISCAN(fd)) {
		struct iovec iov = { (void *)buf, len };
		ssize_t r = qsend(fd, &iov, 1, 0, NULL, 0);
		if (r != -2) return r;
	}
	return real_write(fd, buf, len);
}

ssize_t send(int fd, const void *buf, size_t len, int flags)
{
	NEXT(ssize_t, send, int, const void *, size_t, int);
	if (ISCAN(fd)) {
		struct iovec iov = { (void *)buf, len };
		ssize_t r = qsend(fd, &iov, 1, flags, NULL, 0);
		if (r != -2) return r;
	}
	return real_send(fd, buf, len, flags);
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *addr, socklen_t alen)
{
	NEXT(ssize_t, sendto, int, const void *, size_t, int, const struct sockaddr *, socklen_t);
	if (ISCAN(fd)) {
		struct iovec iov = { (void *)buf, len };
		ssize_t r = qsend(fd, &iov, 1, flags, addr, alen);
		if (r != -2) return r;
	}
	return real_sendto(fd, buf, len, flags, addr, alen);
}

ssize_t sendmsg(int fd, const struct msghdr *msg, int flags)
{
	NEXT(ssize_t, sendmsg, int, const struct msghdr *, int);
	if (ISCAN(fd)) {
		ssize_t r = qsend(fd, msg->msg_iov, msg->msg_iovlen, flags, msg->msg_name, msg->msg_namelen);
		if (r != -2) return r;
	}
	return real_sendmsg(fd, msg, flags);
}
