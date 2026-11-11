/*
 * qcand -- the CAN bus daemon of qnx-socketcan.
 *
 * QNX 8.0's network stack has no PF_CAN, so there is no kernel to carry CAN
 * frames between CAN sockets. qcand is that part in user space: it holds the
 * CAN interfaces (virtual buses, like Linux's vcan) and every CAN socket of
 * libqsocketcan is a connection to it. A frame a socket sends goes to every
 * other socket on the same interface whose filters accept it, with the rules
 * of Linux's CAN_RAW: CAN_RAW_LOOPBACK of the sender, CAN_RAW_RECV_OWN_MSGS,
 * CAN_RAW_FD_FRAMES, CAN_RAW_FILTER / CAN_RAW_JOIN_FILTERS and
 * CAN_RAW_ERR_FILTER of the receiver.
 *
 *   qcand [-f] [-s <socket path>] [<interface>[:fd] ...]
 *
 * Without interfaces it creates vcan0 (CAN FD capable, as Linux's vcan).
 * "<name>:fd" / "<name>:classic" picks the MTU (72 / 16). -f stays in the
 * foreground. More interfaces at run time: qcanctl add / del / list.
 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include "qcan_proto.h"

#define MAXIF      32
#define MAXCLIENT  256
#define MAXFILTER  CAN_RAW_FILTER_MAX

struct iface { int used; char name[QCAN_IFNAMSIZ]; uint32_t mtu; };
struct client {
	int fd;
	int bound;                 /* QCAN_BIND seen */
	uint32_t ifindex;          /* 0: every interface */
	int loopback, recv_own, fd_frames, join, meta;
	uint32_t err_mask;
	int nfilter;               /* -1: the default filter (everything but error frames) */
	struct can_filter *filter;
	uint32_t dropcnt;
};

static struct iface ifs[MAXIF];
static struct client cl[MAXCLIENT];
static struct pollfd pfd[MAXCLIENT + 1];
static int ncl;
static const char *sockpath;

static void cleanup(int sig) { (void)sig; if (sockpath) unlink(sockpath); _exit(0); }

static struct iface *if_byindex(uint32_t idx)
{
	if (idx < QCAN_IFBASE || idx >= QCAN_IFBASE + MAXIF || !ifs[idx - QCAN_IFBASE].used) return NULL;
	return &ifs[idx - QCAN_IFBASE];
}
static int if_byname(const char *name)
{
	for (int i = 0; i < MAXIF; i++)
		if (ifs[i].used && !strcmp(ifs[i].name, name)) return i;
	return -1;
}
static int if_add(const char *name, uint32_t mtu)
{
	if (!*name || strlen(name) >= QCAN_IFNAMSIZ) return -EINVAL;
	if (if_byname(name) >= 0) return -EEXIST;
	for (int i = 0; i < MAXIF; i++)
		if (!ifs[i].used) {
			ifs[i].used = 1; ifs[i].mtu = mtu == CAN_MTU ? CAN_MTU : CANFD_MTU;
			strcpy(ifs[i].name, name);
			return i;
		}
	return -ENOSPC;
}

static void reply(int fd, uint32_t type, uint32_t ifindex, uint32_t arg, const void *p, size_t n)
{
	struct qcan_hdr h = { .type = type, .ifindex = ifindex, .arg = arg };
	struct iovec iov[2] = { { &h, sizeof h }, { (void *)p, n } };
	struct msghdr m = { .msg_iov = iov, .msg_iovlen = n ? 2 : 1 };
	sendmsg(fd, &m, MSG_DONTWAIT);
}
static void reply_if(int fd, int i)
{
	reply(fd, QCAN_IFINFO, QCAN_IFBASE + i, ifs[i].mtu, ifs[i].name, strlen(ifs[i].name) + 1);
}

/* Linux's raw_rcv() filter rules. */
static int accepts(const struct client *c, canid_t id)
{
	if (id & CAN_ERR_FLAG)
		return (id & c->err_mask & CAN_ERR_MASK) != 0;
	if (c->nfilter < 0) return 1;
	int hit = 0;
	for (int i = 0; i < c->nfilter; i++) {
		canid_t fid = c->filter[i].can_id, mask = c->filter[i].can_mask;
		int inv = (fid & CAN_INV_FILTER) != 0;
		mask &= ~CAN_INV_FILTER; fid &= ~CAN_INV_FILTER;
		int m = ((id & mask) == (fid & mask)) != inv;
		if (m) hit++;
		else if (c->join) return 0;
		if (m && !c->join) return 1;
	}
	return c->join ? hit == c->nfilter && hit > 0 : 0;
}

static void deliver(struct client *from, uint32_t ifindex, const void *frame, size_t len)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	struct qcan_hdr h = { .type = QCAN_FRAME, .ifindex = ifindex,
	                      .ts_ns = (uint64_t)ts.tv_sec * 1000000000ull + ts.tv_nsec };
	canid_t id; memcpy(&id, frame, sizeof id);
	for (int i = 0; i < ncl; i++) {
		struct client *c = &cl[i];
		if (!c->bound || (c->ifindex && c->ifindex != ifindex)) continue;
		if (c == from ? !c->recv_own : !from->loopback) continue;
		if (len > CAN_MTU && !c->fd_frames) continue;
		if (!accepts(c, id)) continue;
		h.arg = c == from ? QCAN_F_OWN : 0;
		h.dropcnt = c->dropcnt;
		/* The frame as it is; in meta mode the header after it. */
		struct iovec iov[2] = { { (void *)frame, len }, { &h, sizeof h } };
		struct msghdr m = { .msg_iov = iov, .msg_iovlen = c->meta ? 2 : 1 };
		if (sendmsg(c->fd, &m, MSG_DONTWAIT) < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS))
			c->dropcnt++;       /* the receiver does not read fast enough: as a full socket queue on Linux */
	}
}

static void drop_client(int i)
{
	close(cl[i].fd); free(cl[i].filter);
	cl[i] = cl[ncl - 1]; ncl--;
}

/* One message of client i. Returns -1 when the client is gone. */
static int serve(int i)
{
	static union { struct qcan_hdr h; char b[sizeof(struct qcan_hdr) + QCAN_MAXPAYLOAD + 8]; } m;
	struct client *c = &cl[i];
	ssize_t n = recv(c->fd, &m, sizeof m - 1, MSG_DONTWAIT);
	if (n < 0) return errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR ? 0 : -1;
	if (n == CAN_MTU || n == CANFD_MTU) {        /* a frame as it is, for the bound interface */
		struct iface *f = if_byindex(c->ifindex);
		/* Linux fails such a write() with EINVAL / ENXIO; there is no way back here, so the frame is dropped. */
		if (f && (size_t)n <= f->mtu && (n == CAN_MTU || c->fd_frames)) deliver(c, c->ifindex, &m, n);
		return 0;
	}
	if (n < (ssize_t)sizeof m.h) return -1;      /* 0: closed */
	char *p = m.b + sizeof m.h; size_t len = n - sizeof m.h;
	p[len] = 0;
	int k;
	switch (m.h.type) {
	case QCAN_BIND:
		if (m.h.ifindex && !if_byindex(m.h.ifindex)) { reply(c->fd, QCAN_ERR, 0, ENODEV, NULL, 0); break; }
		c->ifindex = m.h.ifindex; c->bound = 1;
		break;
	case QCAN_FILTER:
		k = len / sizeof(struct can_filter);
		if (k > MAXFILTER) k = MAXFILTER;
		free(c->filter); c->filter = NULL; c->nfilter = k;
		if (k) { c->filter = malloc(k * sizeof(struct can_filter)); if (!c->filter) return -1; memcpy(c->filter, p, k * sizeof(struct can_filter)); }
		break;
	case QCAN_OPT: {
		int v = 0; if (len >= sizeof v) memcpy(&v, p, sizeof v);
		switch (m.h.arg) {
		case CAN_RAW_LOOPBACK:      c->loopback = v != 0; break;
		case CAN_RAW_RECV_OWN_MSGS: c->recv_own = v != 0; break;
		case CAN_RAW_FD_FRAMES:     c->fd_frames = v != 0; break;
		case CAN_RAW_JOIN_FILTERS:  c->join = v != 0; break;
		case CAN_RAW_ERR_FILTER:    c->err_mask = (uint32_t)v; break;
		case QCAN_OPT_META:         c->meta = v != 0; break;
		}
		break;
	}
	case QCAN_FRAME: {
		uint32_t idx = m.h.ifindex ? m.h.ifindex : c->ifindex;
		struct iface *f = if_byindex(idx);
		if (!f || (len != CAN_MTU && len != CANFD_MTU) || len > f->mtu) break;   /* the library checked; nothing to tell */
		deliver(c, idx, p, len);
		break;
	}
	case QCAN_IFBYNAME:
		if ((k = if_byname(p)) >= 0) reply_if(c->fd, k); else reply(c->fd, QCAN_ERR, 0, ENODEV, NULL, 0);
		break;
	case QCAN_IFBYINDEX:
		if (if_byindex(m.h.ifindex)) reply_if(c->fd, m.h.ifindex - QCAN_IFBASE); else reply(c->fd, QCAN_ERR, 0, ENXIO, NULL, 0);
		break;
	case QCAN_IFLIST:
		for (k = 0; k < MAXIF; k++) if (ifs[k].used) reply_if(c->fd, k);
		reply(c->fd, QCAN_ERR, 0, 0, NULL, 0);
		break;
	case QCAN_IFADD:
		if ((k = if_add(p, m.h.arg)) >= 0) reply_if(c->fd, k); else reply(c->fd, QCAN_ERR, 0, -k, NULL, 0);
		break;
	case QCAN_IFDEL:
		if ((k = if_byname(p)) < 0) { reply(c->fd, QCAN_ERR, 0, ENODEV, NULL, 0); break; }
		ifs[k].used = 0;
		for (int j = 0; j < ncl; j++)            /* sockets bound to it receive nothing more */
			if (cl[j].ifindex == (uint32_t)(QCAN_IFBASE + k)) cl[j].bound = 0;
		reply(c->fd, QCAN_ERR, 0, 0, NULL, 0);
		break;
	default:
		return -1;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int fg = 0, o;
	sockpath = getenv(QCAN_SOCK_ENV);
	if (!sockpath || !*sockpath) sockpath = QCAN_SOCK_PATH;
	while ((o = getopt(argc, argv, "fs:h")) != -1)
		switch (o) {
		case 'f': fg = 1; break;
		case 's': sockpath = optarg; break;
		default:
			fprintf(stderr, "usage: qcand [-f] [-s <socket path>] [<interface>[:fd|:classic] ...]\n"
			                "  the CAN bus daemon of qnx-socketcan; default interface: vcan0\n");
			return o == 'h' ? 0 : 2;
		}
	if (optind == argc) if_add("vcan0", CANFD_MTU);
	for (; optind < argc; optind++) {
		char name[64]; uint32_t mtu = CANFD_MTU;
		snprintf(name, sizeof name, "%s", argv[optind]);
		char *col = strchr(name, ':');
		if (col) { *col++ = 0; if (!strcmp(col, "classic")) mtu = CAN_MTU; else if (strcmp(col, "fd")) { fprintf(stderr, "qcand: %s: fd or classic\n", argv[optind]); return 2; } }
		int r = if_add(name, mtu);
		if (r < 0) { fprintf(stderr, "qcand: %s: %s\n", name, strerror(-r)); return 1; }
	}

	struct sockaddr_un sa = { .sun_family = AF_UNIX };
	if (strlen(sockpath) >= sizeof sa.sun_path) { fprintf(stderr, "qcand: socket path too long\n"); return 1; }
	strcpy(sa.sun_path, sockpath);
	int ls = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (ls < 0) { perror("qcand: socket"); return 1; }
	/* A daemon already there answers a connect; a stale socket file does not. */
	if (connect(ls, (struct sockaddr *)&sa, sizeof sa) == 0) { fprintf(stderr, "qcand: already running on %s\n", sockpath); return 1; }
	close(ls);
	unlink(sockpath);
	ls = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (ls < 0 || bind(ls, (struct sockaddr *)&sa, sizeof sa) < 0 || listen(ls, 64) < 0) { perror("qcand: bind"); return 1; }
	chmod(sockpath, 0666);
	signal(SIGPIPE, SIG_IGN); signal(SIGTERM, cleanup); signal(SIGINT, cleanup);
	if (!fg && daemon(1, 0) < 0) { perror("qcand: daemon"); return 1; }
	fcntl(ls, F_SETFL, fcntl(ls, F_GETFL) | O_NONBLOCK);

	for (;;) {
		pfd[0].fd = ls; pfd[0].events = POLLIN;
		for (int i = 0; i < ncl; i++) { pfd[i + 1].fd = cl[i].fd; pfd[i + 1].events = POLLIN; }
		int n = ncl;
		if (poll(pfd, n + 1, -1) < 0) { if (errno == EINTR) continue; break; }
		/* A program sets a socket up (socket, bind, filter, options) and expects
		 * the next frame anyone sends to be handled under that setup, as with the
		 * system calls of Linux. So, before any frame of this round: take the new
		 * connections, then every client's pending setup messages (those ahead
		 * of its first frame). */
		for (int fd; (fd = accept(ls, NULL, NULL)) >= 0; ) {
			if (ncl == MAXCLIENT) { close(fd); continue; }
			int sz = 1 << 20;   /* room for bursts: 1 MiB is about 10000 classic frames */
			setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &sz, sizeof sz);
			cl[ncl] = (struct client){ .fd = fd, .loopback = 1, .nfilter = -1 };
			ncl++;
		}
		for (int i = 0; i < ncl; i++)
			for (;;) {
				union { struct qcan_hdr h; char b[CANFD_MTU + 1]; } u;
				ssize_t l = recv(cl[i].fd, &u, sizeof u, MSG_DONTWAIT | MSG_PEEK);
				if (l < (ssize_t)sizeof u.h || l == CANFD_MTU) break;      /* nothing, closed, or a frame */
				struct qcan_hdr h = u.h;
				if (h.type != QCAN_BIND && h.type != QCAN_FILTER && h.type != QCAN_OPT) break;
				if (serve(i) < 0) break;     /* the frame loop below sees the closed connection */
			}
		/* Backwards: drop_client() moves the last client into the freed slot. */
		for (int i = n - 1; i >= 0; i--) {
			if (!(pfd[i + 1].revents & (POLLIN | POLLHUP | POLLERR))) continue;
			/* Up to 64 messages per round, so one busy sender does not starve the others. */
			int r = 0;
			for (int k = 0; k < 64 && r == 0; k++) {
				r = serve(i);
				struct pollfd q = { cl[i].fd, POLLIN, 0 };
				if (r == 0 && poll(&q, 1, 0) <= 0) break;
			}
			if (r < 0) drop_client(i);
		}
	}
	cleanup(0);
	return 0;
}
