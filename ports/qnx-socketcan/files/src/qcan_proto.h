/*
 * qnx-socketcan: the protocol between libqsocketcan (the AF_CAN socket
 * emulation) and qcand (the CAN bus daemon), over an AF_UNIX
 * SOCK_SEQPACKET connection.
 *
 * A message of CAN_MTU (16) or CANFD_MTU (72) bytes is a CAN frame as it is
 * (struct can_frame / canfd_frame): what write() on a CAN socket sends on
 * the bound interface and what read() receives, with no library in between.
 * Every other message is one header and a payload; none of them is 16 or 72
 * bytes long. A socket in "meta" mode (QCAN_OPT_META, set by the library's
 * recvmsg()/recvfrom()) receives its frames with the header appended
 * (40 or 96 bytes): interface, time stamp, flags, drop count.
 */
#ifndef QCAN_PROTO_H
#define QCAN_PROTO_H
#include <stdint.h>

#define QCAN_SOCK_ENV   "QCAN_SOCKET"          /* overrides the path below */
#define QCAN_SOCK_PATH  "/tmp/qcand.sock"
#define QCAN_IFBASE     1000                   /* interface indexes of CAN interfaces start here */
#define QCAN_IFNAMSIZ   16
#define QCAN_MAXPAYLOAD 4104                   /* CAN_RAW_FILTER_MAX filters and 4 bytes */

enum {
	/* client -> daemon */
	QCAN_BIND = 1,     /* ifindex (0: every interface) */
	QCAN_FILTER,       /* payload: struct can_filter[] (none: receive nothing), then 4 bytes (never a 72 byte message) */
	QCAN_OPT,          /* arg: option (CAN_RAW_*, QCAN_OPT_META), payload: int value */
	QCAN_FRAME,        /* a frame for the interface ifindex (sendto()); payload: struct can_frame or canfd_frame */
	QCAN_IFBYNAME,     /* payload: name            -> QCAN_IFINFO or QCAN_ERR */
	QCAN_IFBYINDEX,    /* ifindex                  -> QCAN_IFINFO or QCAN_ERR */
	QCAN_IFLIST,       /*                          -> QCAN_IFINFO per interface, then QCAN_ERR 0 */
	QCAN_IFADD,        /* payload: name; arg: mtu  -> QCAN_IFINFO or QCAN_ERR */
	QCAN_IFDEL,        /* payload: name            -> QCAN_ERR (0: done) */
	/* daemon -> client */
	QCAN_IFINFO = 64,  /* ifindex, arg: mtu, payload: name */
	QCAN_ERR           /* arg: errno */
};

#define QCAN_OPT_META 0x10000  /* QCAN_OPT: frames to this client carry the header after the frame */
#define QCAN_F_OWN 0x1   /* a frame to a client in meta mode: the client sent this frame itself */

struct qcan_hdr {
	uint32_t type;
	uint32_t ifindex;
	uint64_t ts_ns;     /* after a frame (meta mode): CLOCK_REALTIME when the daemon took the frame */
	uint32_t arg;       /* see the types; after a frame: QCAN_F_* */
	uint32_t dropcnt;   /* after a frame: frames dropped for this client so far (its queue was full) */
};
#endif
