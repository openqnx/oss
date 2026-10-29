/* libcoap: create a context, a resource and a UDP endpoint. */
#include "common.h"
#include <coap3/coap.h>
int main(void) {
  QT_BEGIN("libcoap");
  coap_startup();
  coap_context_t *ctx = coap_new_context(NULL);
  QT_CHECK(ctx != NULL, "context");
  coap_address_t a; coap_address_init(&a);
  a.addr.sin.sin_family = AF_INET; a.addr.sin.sin_port = htons(15683); a.addr.sin.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  a.size = sizeof(a.addr.sin);
  QT_CHECK(coap_new_endpoint(ctx, &a, COAP_PROTO_UDP) != NULL, "UDP endpoint");
  coap_resource_t *r = coap_resource_init(coap_make_str_const("t"), 0);
  QT_CHECK(r != NULL, "resource"); coap_add_resource(ctx, r);
  QT_CHECK(coap_dtls_is_supported(), "DTLS (OpenSSL) available");
  coap_free_context(ctx); coap_cleanup();
  QT_END();
}
