#include "common.h"
#include <pcap.h>
int main(void) {
  QT_BEGIN("libpcap");
  char err[PCAP_ERRBUF_SIZE] = {0};
  const char *v = pcap_lib_version();
  QT_CHECK(v != NULL && v[0] != '\0', "pcap_lib_version");
  pcap_if_t *devs = NULL;
  /* No interfaces is a valid outcome on a bare target; a hard error is not. */
  int rc = pcap_findalldevs(&devs, err);
  QT_CHECK(rc == 0, "pcap_findalldevs succeeds");
  if (rc == 0) { for (pcap_if_t *d = devs; d; d = d->next) printf("  info iface %s\n", d->name); pcap_freealldevs(devs); }
  else printf("  info pcap error: %s\n", err);
  printf("  info %s\n", v);
  QT_END();
}
