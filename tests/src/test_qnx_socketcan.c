/* qnx-socketcan: Linux SocketCAN (PF_CAN, CAN_RAW) on QNX, in a program
 * built the way a Linux CAN program is (<linux/can.h>, socket/bind/read/
 * write/setsockopt/recvmsg/ioctl), linked with libqsocketcan.
 * The test starts a CAN bus daemon of its own (qcand, private socket) with
 * a CAN FD bus tcan0 and a classic bus tcan1, and checks what Linux's
 * CAN_RAW does on vcan interfaces:
 *  - interface lookup (if_nametoindex, SIOCGIFINDEX, SIOCGIFMTU 72 / 16);
 *  - a frame written by one socket arrives unchanged on the others of the
 *    bus, not on the sender, not on the other bus;
 *  - CAN_RAW_RECV_OWN_MSGS (own frame, flagged MSG_CONFIRM), SO_TIMESTAMP;
 *  - CAN_RAW_FILTER: id/mask, CAN_INV_FILTER, an empty set, JOIN_FILTERS;
 *  - CAN_RAW_LOOPBACK off: no local socket receives;
 *  - CAN FD: 72-byte frames need CAN_RAW_FD_FRAMES on both ends and a CAN FD
 *    bus; a classic-only receiver does not get them;
 *  - a socket bound to "any" (index 0) names the bus in recvfrom(), and
 *    sendto() picks one;
 *  - the errors: EPROTONOSUPPORT (CAN_BCM), ENODEV, EINVAL, ENXIO, EAGAIN,
 *    ENETDOWN without a daemon;
 *  - ordinary descriptors (a pipe, a UDP socket) work through the same
 *    read/write/sendto/recvfrom as before. */
#include "common.h"
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <linux/can.h>
#include <linux/can/raw.h>

static int cansock(unsigned ifindex)
{
  int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  struct sockaddr_can a; memset(&a, 0, sizeof a);
  a.can_family = AF_CAN; a.can_ifindex = (int)ifindex;
  if (s >= 0 && bind(s, (struct sockaddr *)&a, sizeof a) < 0) { close(s); return -1; }
  return s;
}
static int ready(int s, int ms) { struct pollfd p = { s, POLLIN, 0 }; return poll(&p, 1, ms) == 1; }
static int put(int s, canid_t id, int len, const char *data)
{
  struct can_frame f; memset(&f, 0, sizeof f);
  f.can_id = id; f.can_dlc = len; memcpy(f.data, data, len);
  return write(s, &f, sizeof f) == sizeof f;
}
/* The next frame's id, or 0xFFFFFFFF when none comes within 300 ms. */
static canid_t get(int s)
{
  struct can_frame f;
  if (!ready(s, 300) || read(s, &f, sizeof f) != sizeof f) return 0xFFFFFFFFu;
  return f.can_id;
}
#define NONE 0xFFFFFFFFu

int main(void) {
  QT_BEGIN("qnx-socketcan");
  char path[64]; snprintf(path, sizeof path, "/tmp/qcand-test.%d", (int)getpid());
  setenv("QCAN_SOCKET", path, 1);

  /* Which read() does this program call: the library's (the library is ahead
   * of libc in the link order) or libc's? Both orders must work. */
  Dl_info di; void *rd = dlsym(RTLD_DEFAULT, "read");
  int exact = rd && dladdr(rd, &di) && di.dli_fname && strstr(di.dli_fname, "qsocketcan");
  printf("  note the program's read() is %s\n", exact ? "libqsocketcan's" : "libc's (libc is ahead of libqsocketcan)");
  fflush(stdout);

  int s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  QT_CHECK(s < 0 && errno == ENETDOWN, "no daemon: socket(PF_CAN) fails with ENETDOWN");

  pid_t d = fork();
  if (d == 0) { execl("/usr/local/bin/qcand", "qcand", "-f", "-s", path, "tcan0", "tcan1:classic", (char *)NULL); _exit(127); }
  unsigned i0 = 0, i1 = 0;
  for (int i = 0; i < 100 && !(i0 = if_nametoindex("tcan0")); i++) usleep(50000);
  i1 = if_nametoindex("tcan1");
  QT_CHECK(i0 >= 1000 && i1 >= 1000 && i0 != i1, "qcand is up; if_nametoindex finds tcan0 and tcan1");
  char nb[IF_NAMESIZE] = "";
  QT_CHECK(if_indextoname(i1, nb) && !strcmp(nb, "tcan1"), "if_indextoname");
  QT_CHECK(if_nametoindex("lo0") > 0 && if_nametoindex("lo0") < 1000 && if_nametoindex("nocan9") == 0, "if_nametoindex still finds lo0, and no nocan9");

  errno = 0;
  QT_CHECK(socket(PF_CAN, SOCK_DGRAM, 2 /* CAN_BCM */) < 0 && errno == EPROTONOSUPPORT, "CAN_BCM: EPROTONOSUPPORT");
  int a = cansock(i0), b = cansock(i0), c = cansock(i1);
  QT_CHECK(a >= 0 && b >= 0 && c >= 0, "three CAN_RAW sockets bound (two on tcan0, one on tcan1)");
  errno = 0;
  QT_CHECK(cansock(4242) < 0 && errno == ENODEV, "bind to an unknown interface index: ENODEV");

  struct ifreq ifr; memset(&ifr, 0, sizeof ifr); strcpy(ifr.ifr_name, "tcan0");
  QT_CHECK(ioctl(a, SIOCGIFINDEX, &ifr) == 0 && (unsigned)ifr.ifr_ifindex == i0, "ioctl SIOCGIFINDEX");
  QT_CHECK(ioctl(a, SIOCGIFMTU, &ifr) == 0 && ifr.ifr_mtu == CANFD_MTU, "tcan0 MTU is 72 (CAN FD)");
  strcpy(ifr.ifr_name, "tcan1");
  QT_CHECK(ioctl(a, SIOCGIFMTU, &ifr) == 0 && ifr.ifr_mtu == CAN_MTU, "tcan1 MTU is 16 (classic)");
  strcpy(ifr.ifr_name, "nocan9"); errno = 0;
  QT_CHECK(ioctl(a, SIOCGIFINDEX, &ifr) < 0 && errno == ENODEV, "SIOCGIFINDEX of an unknown name: ENODEV");

  /* One frame, a -> b. */
  QT_CHECK(put(a, 0x123, 4, "\xDE\xAD\xBE\xEF"), "write a classic frame (16 bytes)");
  struct can_frame f; memset(&f, 0xFF, sizeof f);
  QT_CHECK(ready(b, 1000) && read(b, &f, sizeof f) == sizeof f && f.can_id == 0x123 && f.can_dlc == 4 && !memcmp(f.data, "\xDE\xAD\xBE\xEF", 4),
           "the other socket of the bus reads 123#DEADBEEF");
  QT_CHECK(!ready(a, 200), "the sender does not receive its own frame");
  QT_CHECK(!ready(c, 0), "the socket on the other bus receives nothing");
  struct timeval tv = { 0, 0 };
  QT_CHECK(ioctl(b, SIOCGSTAMP, &tv) == 0 && llabs((long long)tv.tv_sec - (long long)time(NULL)) < 5, "SIOCGSTAMP: the frame's time");
  QT_CHECK(put(a, 0x1ABCDEF0 | CAN_EFF_FLAG, 8, "\1\2\3\4\5\6\7\10") && get(b) == (0x1ABCDEF0 | CAN_EFF_FLAG), "an extended frame keeps its 29-bit id and flag");
  QT_CHECK(put(a, 0x456 | CAN_RTR_FLAG, 0, "") && get(b) == (0x456 | CAN_RTR_FLAG), "a remote frame");

  /* Own frames and the timestamp. */
  int on = 1, off = 0;
  QT_CHECK(setsockopt(a, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &on, sizeof on) == 0 && setsockopt(a, SOL_SOCKET, SO_TIMESTAMP, &on, sizeof on) == 0,
           "setsockopt CAN_RAW_RECV_OWN_MSGS, SO_TIMESTAMP");
  struct sockaddr_can from; struct iovec iov = { &f, sizeof f }; char ctl[128];
  struct msghdr m; memset(&m, 0, sizeof m);
  /* With libc ahead of the library, qcand tells the interface, time and "own"
   * of the frames that arrive after the socket's first recvmsg()/recvfrom(). */
  if (!exact) { m.msg_iov = &iov; m.msg_iovlen = 1; recvmsg(a, &m, MSG_DONTWAIT); memset(&m, 0, sizeof m); }
  put(a, 0x321, 1, "Z");
  m.msg_name = &from; m.msg_namelen = sizeof from; m.msg_iov = &iov; m.msg_iovlen = 1; m.msg_control = ctl; m.msg_controllen = sizeof ctl;
  ssize_t n = ready(a, 1000) ? recvmsg(a, &m, 0) : -1;
  QT_CHECK(n == sizeof f && f.can_id == 0x321 && (m.msg_flags & MSG_CONFIRM) && from.can_family == AF_CAN && (unsigned)from.can_ifindex == i0,
           "recvmsg: the own frame, flagged MSG_CONFIRM, from tcan0");
  int stamped = 0;
  for (struct cmsghdr *cm = CMSG_FIRSTHDR(&m); cm; cm = CMSG_NXTHDR(&m, cm))
    if (cm->cmsg_level == SOL_SOCKET && cm->cmsg_type == SCM_TIMESTAMP) {
      memcpy(&tv, CMSG_DATA(cm), sizeof tv);
      stamped = llabs((long long)tv.tv_sec - (long long)time(NULL)) < 5;
    }
  QT_CHECK(stamped, "recvmsg: an SCM_TIMESTAMP control message with the current time");
  iov.iov_len = sizeof f; m.msg_controllen = sizeof ctl; m.msg_namelen = sizeof from;
  n = ready(b, 1000) ? recvmsg(b, &m, 0) : -1;
  QT_CHECK(n == sizeof f && f.can_id == 0x321 && !(m.msg_flags & MSG_CONFIRM), "the other socket gets it without MSG_CONFIRM");
  setsockopt(a, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &off, sizeof off);
  int v = -1; socklen_t vl = sizeof v;
  QT_CHECK(getsockopt(a, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &v, &vl) == 0 && v == 0 && getsockopt(a, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &v, &vl) == 0 && v == 1,
           "getsockopt: RECV_OWN_MSGS 0, LOOPBACK 1 (the default)");

  /* Filters on b. */
  struct can_filter flt[2] = { { 0x200, 0x700 }, { 0x555, CAN_SFF_MASK } };
  QT_CHECK(setsockopt(b, SOL_CAN_RAW, CAN_RAW_FILTER, flt, sizeof flt) == 0, "setsockopt CAN_RAW_FILTER (200/700, 555/7FF)");
  put(a, 0x123, 0, ""); put(a, 0x205, 0, ""); put(a, 0x554, 0, ""); put(a, 0x555, 0, ""); put(a, 0x2FF, 0, "");
  canid_t g1 = get(b), g2 = get(b), g3 = get(b), g4 = get(b);
  QT_CHECK(g1 == 0x205 && g2 == 0x555 && g3 == 0x2FF && g4 == NONE, "of 123 205 554 555 2FF the filter passes 205 555 2FF");
  struct can_filter inv = { 0x200 | CAN_INV_FILTER, 0x700 };
  setsockopt(b, SOL_CAN_RAW, CAN_RAW_FILTER, &inv, sizeof inv);
  put(a, 0x205, 0, ""); put(a, 0x123, 0, "");
  g1 = get(b); g2 = get(b);
  QT_CHECK(g1 == 0x123 && g2 == NONE, "CAN_INV_FILTER: of 205 123 only 123");
  struct can_filter jn[2] = { { 0x200, 0x700 }, { 0x005, 0x00F } };
  setsockopt(b, SOL_CAN_RAW, CAN_RAW_FILTER, jn, sizeof jn);
  setsockopt(b, SOL_CAN_RAW, CAN_RAW_JOIN_FILTERS, &on, sizeof on);
  put(a, 0x205, 0, ""); put(a, 0x206, 0, ""); put(a, 0x305, 0, ""); put(a, 0x2F5, 0, "");
  g1 = get(b); g2 = get(b); g3 = get(b);
  QT_CHECK(g1 == 0x205 && g2 == 0x2F5 && g3 == NONE, "CAN_RAW_JOIN_FILTERS: of 205 206 305 2F5 both filters match 205 2F5");
  setsockopt(b, SOL_CAN_RAW, CAN_RAW_JOIN_FILTERS, &off, sizeof off);
  setsockopt(b, SOL_CAN_RAW, CAN_RAW_FILTER, NULL, 0);
  put(a, 0x123, 0, "");
  QT_CHECK(get(b) == NONE, "an empty filter set: nothing is received");
  struct can_filter all = { 0, 0 };
  setsockopt(b, SOL_CAN_RAW, CAN_RAW_FILTER, &all, sizeof all);
  put(a, 0x124, 0, "");
  QT_CHECK(get(b) == 0x124, "filter 0/0: everything again");

  /* Loopback off. */
  setsockopt(a, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &off, sizeof off);
  put(a, 0x111, 0, "");
  QT_CHECK(get(b) == NONE, "CAN_RAW_LOOPBACK off: local sockets do not receive the frame");
  setsockopt(a, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &on, sizeof on);

  /* CAN FD. */
  struct canfd_frame fd, rfd; memset(&fd, 0, sizeof fd);
  fd.can_id = 0x7FF; fd.len = 12; fd.flags = CANFD_BRS; memcpy(fd.data, "\x11\x22\x33\x44\x55\x66\x77\x88\x99\xAA\xBB\xCC", 12);
  errno = 0;
  QT_CHECK(send(a, &fd, sizeof fd, 0) < 0 && errno == EINVAL, "send: a CAN FD frame without CAN_RAW_FD_FRAMES: EINVAL");
  /* write(): the same from the library's; libc's hands the frame to qcand, which drops it. */
  errno = 0;
  if (exact) QT_CHECK(write(a, &fd, sizeof fd) < 0 && errno == EINVAL, "write: the same");
  else QT_CHECK(write(a, &fd, sizeof fd) == sizeof fd && !ready(b, 200), "libc's write: qcand drops the frame");
  int e = cansock(i0);
  QT_CHECK(setsockopt(a, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &on, sizeof on) == 0 && setsockopt(e, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &on, sizeof on) == 0,
           "setsockopt CAN_RAW_FD_FRAMES on the sender and one receiver");
  QT_CHECK(write(a, &fd, sizeof fd) == sizeof fd, "write a CAN FD frame (72 bytes)");
  memset(&rfd, 0, sizeof rfd);
  QT_CHECK(ready(e, 1000) && read(e, &rfd, sizeof rfd) == sizeof rfd && rfd.can_id == 0x7FF && rfd.len == 12 && (rfd.flags & CANFD_BRS) && !memcmp(rfd.data, fd.data, 12),
           "the CAN FD receiver reads 7FF##1 with its 12 bytes");
  QT_CHECK(!ready(b, 200), "the classic-only receiver does not get the CAN FD frame");
  put(a, 0x125, 2, "ab");
  QT_CHECK(ready(e, 1000) && read(e, &rfd, sizeof rfd) == sizeof(struct can_frame) && get(b) == 0x125, "a classic frame arrives as 16 bytes on both");
  int cfd = cansock(i1); setsockopt(cfd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &on, sizeof on);
  errno = 0;
  QT_CHECK(send(cfd, &fd, sizeof fd, 0) < 0 && errno == EINVAL, "send: a CAN FD frame on the classic bus tcan1: EINVAL");
  errno = 0;
  if (exact) QT_CHECK(write(cfd, &fd, sizeof fd) < 0 && errno == EINVAL, "write: the same");
  else QT_CHECK(write(cfd, &fd, sizeof fd) == sizeof fd && !ready(c, 200), "libc's write: qcand drops the frame");

  /* Any-interface socket. */
  int any = cansock(0);
  struct sockaddr_can sa; socklen_t sl = sizeof sa; memset(&sa, 0, sizeof sa);
  if (!exact) recvfrom(any, &f, sizeof f, MSG_DONTWAIT, NULL, NULL);
  put(cfd, 0x700, 0, "");
  n = ready(any, 1000) ? recvfrom(any, &f, sizeof f, 0, (struct sockaddr *)&sa, &sl) : -1;
  QT_CHECK(n == sizeof f && f.can_id == 0x700 && (unsigned)sa.can_ifindex == i1, "a socket bound to any interface: recvfrom names tcan1");
  QT_CHECK(get(c) == 0x700, "and tcan1's own socket got the frame too");
  memset(&f, 0, sizeof f); f.can_id = 0x701; errno = 0;
  QT_CHECK(send(any, &f, sizeof f, 0) < 0 && errno == ENXIO, "send on the any-interface socket: ENXIO");
  errno = 0;
  if (exact) QT_CHECK(write(any, &f, sizeof f) < 0 && errno == ENXIO, "write: the same");
  else QT_CHECK(write(any, &f, sizeof f) == sizeof f && !ready(b, 200) && !ready(c, 0), "libc's write: qcand drops the frame");
  sa.can_family = AF_CAN; sa.can_ifindex = (int)i0;
  QT_CHECK(sendto(any, &f, sizeof f, 0, (struct sockaddr *)&sa, sizeof sa) == sizeof f && get(b) == 0x701 && !ready(c, 0), "sendto tcan0: arrives on tcan0 only");

  /* Errors. */
  errno = 0;
  QT_CHECK(send(a, &f, 5, 0) < 0 && errno == EINVAL, "a send of 5 bytes: EINVAL");
  errno = 0;
  if (exact) QT_CHECK(write(a, &f, 5) < 0 && errno == EINVAL, "a write of 5 bytes: EINVAL");
  else QT_CHECK(write(a, &f, 5) == 5 && !ready(b, 200), "libc's write of 5 bytes: qcand drops it");
  fcntl(c, F_SETFL, fcntl(c, F_GETFL) | O_NONBLOCK); errno = 0;
  QT_CHECK(read(c, &f, sizeof f) < 0 && (errno == EAGAIN || errno == EWOULDBLOCK), "a non-blocking read with no frame: EAGAIN");
  int nb2 = socket(PF_CAN, SOCK_RAW | SOCK_NONBLOCK, CAN_RAW); errno = 0;
  QT_CHECK(nb2 >= 0 && (fcntl(nb2, F_GETFL) & O_NONBLOCK), "SOCK_NONBLOCK gives a non-blocking socket");

  /* qcanctl. */
  QT_CHECK(system("/usr/local/bin/qcanctl add tcan2 classic >/dev/null") == 0 && if_nametoindex("tcan2") >= 1000, "qcanctl add tcan2: the interface is there");
  QT_CHECK(system("/usr/local/bin/qcanctl list | grep -q 'tcan0 .*mtu 72'") == 0, "qcanctl list shows tcan0 with mtu 72");
  QT_CHECK(system("/usr/local/bin/qcanctl del tcan2 >/dev/null") == 0 && if_nametoindex("tcan2") == 0, "qcanctl del tcan2: gone");

  /* Ordinary descriptors through the same functions. */
  int p[2]; char buf[16] = "";
  QT_CHECK(pipe(p) == 0 && write(p[1], "pipe", 5) == 5 && read(p[0], buf, sizeof buf) == 5 && !strcmp(buf, "pipe"), "a pipe still reads and writes");
  int u1 = socket(AF_INET, SOCK_DGRAM, 0), u2 = socket(AF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in in; memset(&in, 0, sizeof in); in.sin_family = AF_INET; in.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  socklen_t il = sizeof in;
  int uok = bind(u1, (struct sockaddr *)&in, sizeof in) == 0 && getsockname(u1, (struct sockaddr *)&in, &il) == 0
         && sendto(u2, "udp", 4, 0, (struct sockaddr *)&in, sizeof in) == 4 && ready(u1, 1000) && recvfrom(u1, buf, sizeof buf, 0, NULL, NULL) == 4 && !strcmp(buf, "udp");
  QT_CHECK(uok, "a UDP socket still sends and receives");

  /* The daemon goes away. */
  kill(d, SIGTERM); waitpid(d, NULL, 0);
  errno = 0;
  n = ready(b, 2000) ? read(b, &f, sizeof f) : -2;
  if (exact) QT_CHECK(n < 0 && n != -2 && errno == ENETDOWN, "qcand stopped: read on a CAN socket fails with ENETDOWN");
  else QT_CHECK(n == 0, "qcand stopped: libc's read on a CAN socket returns 0");
  errno = 0;
  n = recv(b, &f, sizeof f, 0);
  if (!(n < 0 && errno == ENETDOWN)) printf("       recv: %ld, errno %d\n", (long)n, errno);
  QT_CHECK(n < 0 && errno == ENETDOWN, "and recv fails with ENETDOWN");
  QT_CHECK(access(path, F_OK) != 0, "qcand removed its socket");
  unlink(path);
  QT_END();
}
