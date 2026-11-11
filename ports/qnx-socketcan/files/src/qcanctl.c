/*
 * qcanctl -- list, add and remove the CAN interfaces of qcand, the CAN bus
 * daemon of qnx-socketcan (what `ip link add dev vcan0 type vcan` and
 * `ip link show` do for Linux's virtual CAN interfaces).
 *
 *   qcanctl list
 *   qcanctl add <name> [fd|classic]     (default: fd, a CAN FD capable bus)
 *   qcanctl del <name>
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <linux/can.h>
#include "qcan_proto.h"

static int usage(int rc)
{
	fprintf(rc ? stderr : stdout,
	        "usage: qcanctl list\n"
	        "       qcanctl add <name> [fd|classic]\n"
	        "       qcanctl del <name>\n"
	        "  the CAN interfaces of qcand (socket: $%s, default %s)\n", QCAN_SOCK_ENV, QCAN_SOCK_PATH);
	return rc;
}

int main(int argc, char **argv)
{
	if (argc >= 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) return usage(0);
	if (argc < 2) return usage(2);
	uint32_t type, mtu = 0;
	const char *name = NULL;
	if (!strcmp(argv[1], "list") && argc == 2) type = QCAN_IFLIST;
	else if (!strcmp(argv[1], "add") && (argc == 3 || argc == 4)) {
		type = QCAN_IFADD; name = argv[2]; mtu = CANFD_MTU;
		if (argc == 4) { if (!strcmp(argv[3], "classic")) mtu = CAN_MTU; else if (strcmp(argv[3], "fd")) return usage(2); }
	} else if (!strcmp(argv[1], "del") && argc == 3) { type = QCAN_IFDEL; name = argv[2]; }
	else return usage(2);
	if (name && strlen(name) >= QCAN_IFNAMSIZ) { fprintf(stderr, "qcanctl: %s: name too long (%d characters at most)\n", name, QCAN_IFNAMSIZ - 1); return 1; }

	const char *path = getenv(QCAN_SOCK_ENV);
	if (!path || !*path) path = QCAN_SOCK_PATH;
	struct sockaddr_un sa = { .sun_family = AF_UNIX };
	strlcpy(sa.sun_path, path, sizeof sa.sun_path);
	int fd = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (fd < 0 || connect(fd, (struct sockaddr *)&sa, sizeof sa) < 0) { fprintf(stderr, "qcanctl: qcand is not running (%s: %s)\n", path, strerror(errno)); return 1; }

	struct qcan_hdr h = { .type = type, .arg = mtu };
	struct iovec iov[2] = { { &h, sizeof h }, { (void *)name, name ? strlen(name) + 1 : 0 } };
	struct msghdr m = { .msg_iov = iov, .msg_iovlen = name ? 2 : 1 };
	if (sendmsg(fd, &m, 0) < 0) { perror("qcanctl: send"); return 1; }
	for (;;) {
		char nb[QCAN_IFNAMSIZ] = "";
		struct iovec riov[2] = { { &h, sizeof h }, { nb, sizeof nb - 1 } };
		struct msghdr rm = { .msg_iov = riov, .msg_iovlen = 2 };
		ssize_t n = recvmsg(fd, &rm, 0);
		if (n < (ssize_t)sizeof h) { fprintf(stderr, "qcanctl: no answer from qcand\n"); return 1; }
		if (h.type == QCAN_IFINFO) {
			printf("%-15s index %u  mtu %u (%s)\n", nb, h.ifindex, h.arg, h.arg >= CANFD_MTU ? "CAN FD" : "classic CAN");
			if (type == QCAN_IFLIST) continue;
			return 0;
		}
		if (h.type == QCAN_ERR && h.arg == 0) return 0;
		fprintf(stderr, "qcanctl: %s%s%s\n", name ? name : "", name ? ": " : "", strerror(h.type == QCAN_ERR ? (int)h.arg : EPROTO));
		return 1;
	}
}
