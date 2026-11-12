/* libcanard (Cyphal/CAN, OpenCyphal): two nodes (10 and 20) on a CAN bus in
 * memory; every frame a node transmits is recorded and handed to the other.
 *  - a 5-byte message on subject 1234 (Cyphal v1.0, 13-bit) is one classic
 *    frame: id 0x1064D20A (priority nominal, subject 1234, source node 10),
 *    payload + tail byte 0xE7 (start, end, toggle, transfer-ID 7); the
 *    subscriber gets the bytes, source node 10, transfer-ID 7, priority;
 *  - a 100-byte message takes 15 classic frames (7 bytes each + transfer
 *    CRC) and arrives whole; with CAN FD frames it takes 2 (the padding
 *    of the last frame is delivered with the payload, as Cyphal defines);
 *  - a subscription with extent 16 gets the first 16 bytes of it;
 *  - a message on a 16-bit subject (Cyphal v1.1) arrives on its subscriber
 *    and not on the 13-bit subscriber of the same number;
 *  - a service: node 10 requests service 77 of node 20, node 20 responds,
 *    the response reaches node 10; a third node (30) sees neither;
 *  - a UAVCAN v0/DroneCAN broadcast arrives on a v0 subscription;
 *  - a transfer whose deadline has passed is not sent (err.tx_expiration);
 *  - nothing is left allocated when the nodes are destroyed. */
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <canard.h>

static long live_allocs;
static void *m_alloc(canard_mem_t m, size_t n) { (void)m; void *p = malloc(n); if (p) live_allocs++; return p; }
static void m_free(canard_mem_t m, size_t n, void *p) { (void)m; (void)n; if (p) { live_allocs--; free(p); } }
static const canard_mem_vtable_t memvt = { .free = m_free, .alloc = m_alloc };

#define MAXF 128
static struct frame { canard_t *from; uint32_t id; size_t len; uint8_t d[64]; bool fd; } bus[MAXF];
static int nbus, ndone;
static canard_us_t now_us = 1000000;
static bool bus_busy;
static canard_t N10, N20, N30;

static canard_us_t v_now(const canard_t *c) { (void)c; return now_us; }
static bool v_tx(canard_t *c, void *uc, canard_us_t deadline, uint_least8_t iface, bool fd, uint32_t id, canard_bytes_t data) {
  (void)uc; (void)deadline; (void)iface;
  if (bus_busy || nbus >= MAXF) return false;
  bus[nbus].from = c; bus[nbus].id = id; bus[nbus].len = data.size; bus[nbus].fd = fd;
  if (data.size) memcpy(bus[nbus].d, data.data, data.size);
  nbus++; return true;
}
static const canard_vtable_t vt = { .now = v_now, .tx = v_tx, .filter = NULL };

static void spin(int rounds) {
  canard_t *all[3] = { &N10, &N20, &N30 };
  while (rounds-- > 0) {
    now_us += 1000;
    for (int i = 0; i < 3; i++) canard_poll(all[i], 1);
    while (ndone < nbus) {
      struct frame *f = &bus[ndone++];
      for (int i = 0; i < 3; i++)
        if (all[i] != f->from) canard_ingest_frame(all[i], now_us, 0, f->id, (canard_bytes_t){ f->len, f->d });
    }
  }
}

struct rx { int count; size_t size; uint8_t data[256]; uint_least8_t src, tid; canard_prio_t prio; };
static void on_msg(canard_subscription_t *s, canard_us_t ts, canard_prio_t prio, uint_least8_t src, uint_least8_t tid, canard_payload_t p) {
  struct rx *r = s->user_context; (void)ts;
  r->count++; r->size = p.view.size; r->src = src; r->tid = tid; r->prio = prio;
  if (p.view.size <= sizeof r->data && p.view.size) memcpy(r->data, p.view.data, p.view.size);
  if (p.origin.data) s->owner->mem.rx_payload.vtable->free(s->owner->mem.rx_payload, p.origin.size, p.origin.data);
}
static const canard_subscription_vtable_t subvt = { .on_message = on_msg };
static canard_bytes_chain_t pl(const void *d, size_t n) { return (canard_bytes_chain_t){ .bytes = { n, d }, .next = NULL }; }

int main(void) {
  QT_BEGIN("libcanard");
  canard_mem_t m = { &memvt, NULL };
  canard_mem_set_t ms = { m, m, m, m, m };
  QT_CHECK(canard_new(&N10, &vt, ms, 1, 64, 0x1111, 0) && canard_new(&N20, &vt, ms, 1, 64, 0x2222, 0) && canard_new(&N30, &vt, ms, 1, 64, 0x3333, 0), "canard_new: three nodes");
  QT_CHECK(canard_set_node_id(&N10, 10) && canard_set_node_id(&N20, 20) && canard_set_node_id(&N30, 30), "node-IDs 10, 20, 30");
  QT_CHECK(!canard_set_node_id(&N10, CANARD_NODE_ID_ANONYMOUS), "the anonymous node-ID is refused");
  N10.tx.fd = N20.tx.fd = N30.tx.fd = false;
  uint8_t msg[256]; for (unsigned i = 0; i < sizeof msg; i++) msg[i] = (uint8_t)(i * 3 + 5);

  static struct rx r13, r13small, r16, rreq, rresp, r30req, r30resp, rv0;
  static canard_subscription_t s13, s13small, s16, sreq, sresp, s30req, s30resp, sv0;
  QT_CHECK(canard_subscribe_13b(&N20, &s13, 1234, 200, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_us, &subvt) == &s13, "node 20 subscribes to subject 1234 (13-bit)");
  s13.user_context = &r13;
  QT_CHECK(canard_find_subscription(&N20, canard_kind_message_13b, 1234) == &s13 && canard_find_subscription(&N20, canard_kind_message_13b, 1235) == NULL, "canard_find_subscription");

  QT_CHECK(canard_publish_13b(&N10, now_us + 1000000, 1, canard_prio_nominal, 1234, 7, pl(msg, 5), NULL), "node 10 publishes 5 bytes, transfer-ID 7");
  spin(3);
  QT_CHECK(nbus == 1, "one frame on the bus");
  printf("  info frame id 0x%08X len %u tail 0x%02X\n", (unsigned)bus[0].id, (unsigned)bus[0].len, bus[0].d[bus[0].len ? bus[0].len - 1 : 0]);
  QT_CHECK(bus[0].id == 0x1064D20AU, "CAN id 0x1064D20A: priority nominal, subject 1234, source node 10");
  QT_CHECK(bus[0].len == 6 && memcmp(bus[0].d, msg, 5) == 0 && bus[0].d[5] == 0xE7, "payload + tail byte 0xE7 (start, end, toggle, transfer-ID 7)");
  QT_CHECK(r13.count == 1 && r13.size == 5 && memcmp(r13.data, msg, 5) == 0, "the subscriber receives the 5 bytes");
  QT_CHECK(r13.src == 10 && r13.tid == 7 && r13.prio == canard_prio_nominal, "source node 10, transfer-ID 7, priority nominal");

  int before = nbus;
  QT_CHECK(canard_publish_13b(&N10, now_us + 1000000, 1, canard_prio_high, 1234, 8, pl(msg, 100), NULL), "node 10 publishes 100 bytes");
  spin(5);
  printf("  info 100 bytes: %d classic frames\n", nbus - before);
  QT_CHECK(nbus - before == 15, "15 classic frames (100 bytes + 2 bytes CRC, 7 per frame)");
  QT_CHECK(r13.count == 2 && r13.size == 100 && memcmp(r13.data, msg, 100) == 0 && r13.prio == canard_prio_high, "the subscriber receives the 100 bytes, priority high");

  canard_subscribe_13b(&N30, &s13small, 1234, 16, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_us, &subvt); s13small.user_context = &r13small;
  N10.tx.fd = true; before = nbus;
  QT_CHECK(canard_publish_13b(&N10, now_us + 1000000, 1, canard_prio_nominal, 1234, 9, pl(msg, 100), NULL), "node 10 publishes 100 bytes with CAN FD frames");
  spin(5);
  QT_CHECK(nbus - before == 2 && bus[before].fd && bus[before].len == 64, "2 CAN FD frames, the first 64 bytes long");
  printf("  info CAN FD: second frame %u bytes, received payload %u bytes\n", (unsigned)bus[before + 1].len, (unsigned)r13.size);
  /* CAN FD frame lengths are 8, 12, 16, 20, 24, 32, 48, 64: the last frame is padded and Cyphal
   * delivers the padding with the payload (the data type's length says where it ends). */
  QT_CHECK(r13.count == 3 && r13.size >= 100 && r13.size < 112 && memcmp(r13.data, msg, 100) == 0, "the subscriber receives the 100 bytes (and the padding of the last frame)");
  QT_CHECK(r13small.count == 1 && r13small.size == 16 && memcmp(r13small.data, msg, 16) == 0, "a subscription with extent 16 gets the first 16 bytes");
  N10.tx.fd = false;

  QT_CHECK(canard_subscribe_16b(&N20, &s16, 1234, 64, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_us, &subvt) == &s16, "node 20 subscribes to subject 1234 (16-bit, Cyphal v1.1)");
  s16.user_context = &r16;
  QT_CHECK(canard_publish_16b(&N10, now_us + 1000000, 1, canard_prio_nominal, 1234, 1, pl("v1.1", 4), NULL), "node 10 publishes on the 16-bit subject");
  spin(3);
  QT_CHECK(r16.count == 1 && r16.size == 4 && memcmp(r16.data, "v1.1", 4) == 0, "the 16-bit subscriber receives it");
  QT_CHECK(r13.count == 3, "the 13-bit subscriber of the same number does not");

  canard_subscribe_request(&N20, &sreq, 77, 64, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_us, &subvt); sreq.user_context = &rreq;
  canard_subscribe_response(&N10, &sresp, 77, 64, &subvt); sresp.user_context = &rresp;
  canard_subscribe_request(&N30, &s30req, 77, 64, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_us, &subvt); s30req.user_context = &r30req;
  canard_subscribe_response(&N30, &s30resp, 77, 64, &subvt); s30resp.user_context = &r30resp;
  QT_CHECK(canard_request(&N10, now_us + 1000000, canard_prio_fast, 77, 20, 3, pl("ping", 4), NULL), "node 10 requests service 77 of node 20");
  spin(3);
  QT_CHECK(rreq.count == 1 && rreq.size == 4 && memcmp(rreq.data, "ping", 4) == 0 && rreq.src == 10 && rreq.tid == 3, "node 20 receives the request from node 10, transfer-ID 3");
  QT_CHECK(canard_respond(&N20, now_us + 1000000, rreq.prio, 77, rreq.src, rreq.tid, pl("pong!", 5), NULL), "node 20 responds");
  spin(3);
  QT_CHECK(rresp.count == 1 && rresp.size == 5 && memcmp(rresp.data, "pong!", 5) == 0 && rresp.src == 20 && rresp.tid == 3, "node 10 receives the response from node 20");
  QT_CHECK(r30req.count == 0 && r30resp.count == 0, "node 30 sees neither the request nor the response");

  uint16_t seed = canard_v0_crc_seed_from_data_type_signature(0x0F0868D0C1A7C6F1ULL);
  QT_CHECK(canard_v0_subscribe(&N20, &sv0, 341, seed, 64, CANARD_DEFAULT_TRANSFER_ID_TIMEOUT_us, &subvt) == &sv0, "node 20 subscribes to the v0 data type 341 (NodeStatus)");
  sv0.user_context = &rv0;
  QT_CHECK(canard_v0_publish(&N10, now_us + 1000000, 1, canard_prio_nominal, 341, seed, 0, pl(msg, 7), NULL), "node 10 broadcasts a v0 NodeStatus (7 bytes)");
  spin(3);
  QT_CHECK(rv0.count == 1 && rv0.size == 7 && memcmp(rv0.data, msg, 7) == 0 && rv0.src == 10, "the v0 subscriber receives it");

  bus_busy = true; before = nbus;
  uint64_t exp0 = N10.err.tx_expiration;
  QT_CHECK(canard_publish_13b(&N10, now_us + 2000, 1, canard_prio_nominal, 1234, 10, pl(msg, 5), NULL), "publish with a 2 ms deadline on a busy bus");
  spin(10);
  bus_busy = false; spin(3);
  QT_CHECK(nbus == before && N10.err.tx_expiration == exp0 + 1 && r13.count == 3, "the transfer expired and was not sent (err.tx_expiration)");
  QT_CHECK(N10.err.oom == 0 && N20.err.oom == 0 && N20.err.rx_transfer == 0, "no out-of-memory or reassembly errors");

  canard_unsubscribe(&N20, &s13); canard_unsubscribe(&N20, &s16); canard_unsubscribe(&N20, &sreq); canard_unsubscribe(&N20, &sv0);
  canard_unsubscribe(&N10, &sresp); canard_unsubscribe(&N30, &s13small); canard_unsubscribe(&N30, &s30req); canard_unsubscribe(&N30, &s30resp);
  canard_destroy(&N10); canard_destroy(&N20); canard_destroy(&N30);
  printf("  info live allocations after destroy: %ld\n", live_allocs);
  QT_CHECK(live_allocs == 0, "nothing is left allocated");
  QT_END();
}
