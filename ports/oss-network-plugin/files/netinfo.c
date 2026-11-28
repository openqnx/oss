/* netinfo: see netinfo.h */
#include "netinfo.h"
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static gboolean skipped(const char *n) {
  static const char *pre[] = { "lo", "enc", "pflog", "pfsync", NULL };
  for (int i = 0; pre[i]; i++) if (!strncmp(n, pre[i], strlen(pre[i]))) return TRUE;
  return FALSE;
}
static NetIf *find_if(NetState *s, const char *name) {
  for (guint i = 0; i < s->ifs->len; i++) {
    NetIf *f = g_ptr_array_index(s->ifs, i);
    if (!strcmp(f->name, name)) return f;
  }
  NetIf *f = g_new0(NetIf, 1);
  g_strlcpy(f->name, name, sizeof f->name);
  f->link = -1;
  f->v4 = g_ptr_array_new_with_free_func(g_free);
  f->v6 = g_ptr_array_new_with_free_func(g_free);
  g_ptr_array_add(s->ifs, f);
  return f;
}
static int prefix_len(const struct sockaddr *m) {
  if (!m) return -1;
  const unsigned char *b; int n;
  if (m->sa_family == AF_INET) { b = (const unsigned char *)&((const struct sockaddr_in *)m)->sin_addr; n = 4; }
  else if (m->sa_family == AF_INET6) { b = (const unsigned char *)&((const struct sockaddr_in6 *)m)->sin6_addr; n = 16; }
  else return -1;
  int p = 0;
  for (int i = 0; i < n; i++) for (int k = 7; k >= 0; k--) if (b[i] & (1 << k)) p++;
  return p;
}

/* default IPv4 route: RTM_GET for 0.0.0.0/0 on a routing socket */
static void default_route(NetState *s) {
  int fd = socket(PF_ROUTE, SOCK_RAW, AF_INET);
  if (fd < 0) return;
  struct { struct rt_msghdr h; char buf[512]; } m;
  memset(&m, 0, sizeof m);
  struct sockaddr_in *dst = (struct sockaddr_in *)(&m.h + 1), *msk = dst + 1;
  dst->sin_len = msk->sin_len = sizeof *dst;
  dst->sin_family = msk->sin_family = AF_INET;
  m.h.rtm_msglen = sizeof m.h + 2 * sizeof *dst;
  m.h.rtm_version = RTM_VERSION; m.h.rtm_type = RTM_GET;
  m.h.rtm_addrs = RTA_DST | RTA_NETMASK; m.h.rtm_seq = 1; m.h.rtm_pid = getpid();
  struct timeval tv = { 2, 0 };
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  if (write(fd, &m, m.h.rtm_msglen) < 0) { close(fd); return; }  /* ESRCH: no default route */
  ssize_t n;
  do n = read(fd, &m, sizeof m);
  while (n > 0 && (m.h.rtm_seq != 1 || m.h.rtm_pid != getpid()));
  close(fd);
  if (n <= 0 || m.h.rtm_errno) return;
  const char *p = (const char *)(&m.h + 1), *end = (const char *)&m + n;
  for (int i = 0; i < RTAX_MAX && p < end; i++) {
    if (!(m.h.rtm_addrs & (1 << i))) continue;
    const struct sockaddr *sa = (const struct sockaddr *)p;
    if (i == RTAX_GATEWAY && sa->sa_family == AF_INET)
      inet_ntop(AF_INET, &((const struct sockaddr_in *)sa)->sin_addr, s->gw, sizeof s->gw);
    int l = sa->sa_len ? sa->sa_len : (int)sizeof(long);
    p += (l + sizeof(long) - 1) & ~(sizeof(long) - 1);
  }
  if (s->gw[0] && m.h.rtm_index > 0) if_indextoname(m.h.rtm_index, s->gwif);
}

NetState *net_read(void) {
  NetState *s = g_new0(NetState, 1);
  s->ifs = g_ptr_array_new();
  struct ifaddrs *ia = NULL;
  if (getifaddrs(&ia) != 0) return s;   /* no network stack: an empty state */
  s->sock_ok = TRUE;
  for (struct ifaddrs *a = ia; a; a = a->ifa_next) {
    if (!a->ifa_name || skipped(a->ifa_name) || !a->ifa_addr) continue;
    NetIf *f = find_if(s, a->ifa_name);
    f->up = (a->ifa_flags & IFF_UP) != 0;
    f->running = (a->ifa_flags & IFF_RUNNING) != 0;
    int fam = a->ifa_addr->sa_family;
    if (fam == AF_LINK) {
      const struct sockaddr_dl *dl = (const struct sockaddr_dl *)a->ifa_addr;
      const struct if_data *d = a->ifa_data;
      if (d) {
        f->link = d->ifi_link_state == LINK_STATE_UP ? 1 : d->ifi_link_state == LINK_STATE_DOWN ? 0 : -1;
        f->wifi = d->ifi_type == IFT_IEEE80211;
      }
      if (dl->sdl_alen == 6) {
        const unsigned char *m = (const unsigned char *)LLADDR(dl);
        g_snprintf(f->mac, sizeof f->mac, "%02x:%02x:%02x:%02x:%02x:%02x", m[0], m[1], m[2], m[3], m[4], m[5]);
      }
    } else if (fam == AF_INET || fam == AF_INET6) {
      char b[INET6_ADDRSTRLEN] = "";
      const void *ad = fam == AF_INET ? (const void *)&((const struct sockaddr_in *)a->ifa_addr)->sin_addr
                                      : (const void *)&((const struct sockaddr_in6 *)a->ifa_addr)->sin6_addr;
      inet_ntop(fam, ad, b, sizeof b);
      int pl = prefix_len(a->ifa_netmask);
      gchar *t = pl >= 0 ? g_strdup_printf("%s/%d", b, pl) : g_strdup(b);
      g_ptr_array_add(fam == AF_INET ? f->v4 : f->v6, t);
    }
  }
  freeifaddrs(ia);
  default_route(s);
  return s;
}

void net_free(NetState *s) {
  if (!s) return;
  for (guint i = 0; i < s->ifs->len; i++) {
    NetIf *f = g_ptr_array_index(s->ifs, i);
    g_ptr_array_free(f->v4, TRUE); g_ptr_array_free(f->v6, TRUE); g_free(f);
  }
  g_ptr_array_free(s->ifs, TRUE);
  g_free(s);
}

NetStatus net_status(const NetState *s, const NetIf **primary) {
  const NetIf *p = NULL;
  for (guint i = 0; i < s->ifs->len && !p; i++) {
    const NetIf *f = g_ptr_array_index(s->ifs, i);
    if (s->gwif[0] && !strcmp(f->name, s->gwif)) p = f;
  }
  for (guint i = 0; i < s->ifs->len && !p; i++) {
    const NetIf *f = g_ptr_array_index(s->ifs, i);
    if (f->up && f->link != 0 && f->v4->len) p = f;
  }
  if (!p && s->ifs->len) p = g_ptr_array_index(s->ifs, 0);
  if (primary) *primary = p;
  if (!p) return NET_NONE;
  if (!p->up || !p->running || p->link == 0 || (!p->v4->len && !p->v6->len)) return NET_DOWN;
  if (!s->gw[0]) return NET_NOROUTE;
  return NET_OK;
}

const char *net_status_text(NetStatus st) {
  switch (st) {
  case NET_OK: return "connected";
  case NET_NOROUTE: return "connected, no default route";
  case NET_DOWN: return "disconnected";
  default: return "no network";
  }
}

int net_events_open(void) {
  int fd = socket(PF_ROUTE, SOCK_RAW, AF_UNSPEC);
  if (fd < 0) return -1;
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
  fcntl(fd, F_SETFD, FD_CLOEXEC);
  return fd;
}
void net_events_drain(int fd) {
  char buf[2048];
  while (read(fd, buf, sizeof buf) > 0) ;
}

static const char *argv_buf[3][6];
static char ifn_buf[3][IFNAMSIZ];
static const char *const *mk(int k, const char *a0, const char *a1, const char *a2, const char *ifname, int at) {
  g_strlcpy(ifn_buf[k], ifname, IFNAMSIZ);
  const char **v = argv_buf[k];
  v[0] = a0; v[1] = a1; v[2] = a2; v[3] = NULL; v[4] = NULL;
  v[at] = ifn_buf[k];
  return v;
}
/* dhcpcd -U answers anyone (through its unprivileged socket). The renewal is
 * -N (renew the interface's addresses), not -n: -n also makes the running
 * dhcpcd reload its configuration -- from the default file, since the
 * command line carries no -f -- and it then took on every interface (lo0,
 * enc0, pflog0, pfsync0 brought up on the QNX image). -N needs root. */
const char *const *net_cmd_lease(const char *ifname) { return mk(0, "dhcpcd", "-U", NULL, ifname, 2); }
const char *const *net_cmd_renew(const char *ifname) { return mk(1, "dhcpcd", "-N", NULL, ifname, 2); }
const char *const *net_cmd_wifi(const char *ifname) {
  const char **v = argv_buf[2];
  g_strlcpy(ifn_buf[2], ifname, IFNAMSIZ);
  v[0] = "wpa_cli"; v[1] = "-i"; v[2] = ifn_buf[2]; v[3] = "status"; v[4] = NULL;
  return v;
}

gchar *net_kv(const char *text, const char *key) {
  if (!text) return NULL;
  size_t kl = strlen(key);
  for (const char *l = text; l && *l; ) {
    const char *e = strchr(l, '\n');
    if (!strncmp(l, key, kl) && l[kl] == '=') {
      gchar *v = g_strndup(l + kl + 1, e ? (gsize)(e - l - kl - 1) : strlen(l + kl + 1));
      /* dhcpcd -U quotes values: 'a b' */
      if (v[0] == '\'' && strlen(v) >= 2 && v[strlen(v) - 1] == '\'') { memmove(v, v + 1, strlen(v)); v[strlen(v) - 1] = 0; }
      return v;
    }
    l = e ? e + 1 : NULL;
  }
  return NULL;
}
