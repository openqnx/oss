/*
 * QNX: <linux/can.h> includes <linux/socket.h>. The socket definitions are
 * QNX's own; what QNX's <sys/socket.h> lacks for SocketCAN is added here.
 *
 * AF_CAN is not a protocol family of io-sock: socket(PF_CAN, ...) works in
 * programs linked with libqsocketcan (qnx-socketcan), which implements CAN
 * sockets on connections to the CAN bus daemon qcand. The number is free in
 * QNX's <sys/socket.h> (AF_MAX is 44) and fits sa_family_t (8 bits there).
 */
#ifndef QNX_LINUX_SOCKET_H
#define QNX_LINUX_SOCKET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/sockio.h>
#include <net/if.h>
/* <linux/can.h>'s struct sockaddr_can starts with this type (as on Linux). */
typedef unsigned short __kernel_sa_family_t;
#ifndef AF_CAN
#define AF_CAN 240
#endif
#ifndef PF_CAN
#define PF_CAN AF_CAN
#endif
/* Linux names of what QNX has under BSD names. */
#ifndef ifr_ifindex
#define ifr_ifindex ifr_index
#endif
/* Linux requests QNX has not: served by libqsocketcan on CAN sockets. */
#ifndef SIOCGSTAMP
#define SIOCGSTAMP  _IOR('q', 1, struct timeval)    /* time of the last frame read */
#endif
#ifndef SIOCGIFNAME
#define SIOCGIFNAME _IOWR('q', 2, struct ifreq)     /* interface name of ifr_ifindex */
#endif
/* Socket options of Linux that candump and friends set on CAN sockets.
 * libqsocketcan implements SO_RXQ_OVFL and accepts the others. */
#ifndef SO_RXQ_OVFL
#define SO_RXQ_OVFL     0x7140
#endif
#ifndef SO_TIMESTAMPING
#define SO_TIMESTAMPING 0x7141
#endif
#ifndef SO_RCVBUFFORCE
#define SO_RCVBUFFORCE  0x7142
#endif
#ifndef SO_PRIORITY
#define SO_PRIORITY     0x7143
#endif
#ifndef SO_MARK
#define SO_MARK         0x7144
#endif
#ifndef SO_TXTIME
#define SO_TXTIME       0x7145
#endif
#ifndef SCM_TXTIME
#define SCM_TXTIME      SO_TXTIME
#endif
#ifndef MSG_CONFIRM
#define MSG_CONFIRM     0x40000000  /* recvmsg() msg_flags: the socket's own frame */
#endif
/* ioctl() on a CAN socket (SIOCGIFINDEX, SIOCGIFMTU, SIOCGSTAMP, ...) is
 * libqsocketcan's work. A program whose link line names libc ahead of
 * libqsocketcan would reach libc's ioctl() instead, so the call is
 * redirected here by name; for other descriptors qcan_ioctl() is ioctl().
 * QCAN_NO_IOCTL_REDIRECT (before this header) leaves ioctl() alone. */
#ifndef QCAN_NO_IOCTL_REDIRECT
#include <sys/ioctl.h>
#ifdef __cplusplus
extern "C" {
#endif
int qcan_ioctl(int __fd, int __cmd, ...);
#ifdef __cplusplus
}
#endif
#define ioctl qcan_ioctl
#endif
#endif
