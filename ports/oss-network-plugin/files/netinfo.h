/* netinfo: the state of the network as the panel shows it, read the BSD way
 * (QNX io-sock): getifaddrs() for interfaces, addresses and link state, a
 * PF_ROUTE socket for the default route and for change events, dhcpcd for
 * the lease, wpa_cli for Wi-Fi. No privileges needed. */
#ifndef NETINFO_H
#define NETINFO_H
#include <glib.h>
#include <net/if.h>

typedef struct {
  char name[IFNAMSIZ];
  gboolean up, running, wifi;
  int link;              /* 1 up, 0 down, -1 unknown */
  char mac[32];
  GPtrArray *v4, *v6;    /* "address/prefix" strings */
} NetIf;

typedef struct {
  GPtrArray *ifs;        /* NetIf*, loopback and pseudo interfaces left out */
  char gw[64];           /* default IPv4 gateway, "" when none */
  char gwif[IFNAMSIZ];   /* its interface */
  gboolean sock_ok;      /* the network stack answered at all */
} NetState;

typedef enum { NET_NONE, NET_DOWN, NET_NOROUTE, NET_OK } NetStatus;

NetState *net_read(void);
void net_free(NetState *s);
/* the interface the panel talks about (that of the default route, else the
 * first with a link and an IPv4 address, else the first), and the status */
NetStatus net_status(const NetState *s, const NetIf **primary);
const char *net_status_text(NetStatus st);
/* a PF_ROUTE socket whose readability means "something changed"; -1 if none */
int net_events_open(void);
void net_events_drain(int fd);
/* argv for: the lease (dhcpcd -U IF), a renewal (dhcpcd -N IF), Wi-Fi status
 * (wpa_cli -i IF status); NULL-terminated, static storage */
const char *const *net_cmd_lease(const char *ifname);
const char *const *net_cmd_renew(const char *ifname);
const char *const *net_cmd_wifi(const char *ifname);
/* "key=value" lines -> value of key (newly allocated) or NULL */
gchar *net_kv(const char *text, const char *key);
#endif
