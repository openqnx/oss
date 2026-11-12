/* isotp-c: ISO 15765-2 between two links that share a CAN bus in memory
 * (isotp_user_send_can records every frame and queues it for the other link,
 * as a bus delivers it: after the sender's call returns; the clock is the
 * test's own).
 *  - a 3-byte message is one Single Frame "03 01 02 03" padded with 0xAA;
 *  - a 20-byte message: First Frame "10 14 ...", the receiver's Flow Control
 *    "30 08 00" (block size 8, STmin 0), Consecutive Frames "21 ...",
 *    "22 ..." -- and the receiver gets the 20 bytes;
 *  - 300 bytes cross in both directions (several Flow Control blocks);
 *  - CAN FD: with TX_DL 64 a 100-byte message takes one First Frame and one
 *    Consecutive Frame;
 *  - errors: a message larger than the send buffer is refused
 *    (ISOTP_RET_OVERFLOW), a second send during a transfer is
 *    ISOTP_RET_INPROGRESS, isotp_receive on an idle link is
 *    ISOTP_RET_NO_DATA, and a First Frame nobody answers ends in
 *    ISOTP_PROTOCOL_RESULT_TIMEOUT_BS once the clock passes the timeout. */
#include "common.h"
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <isotp.h>

#define MAXF 256
static struct { uint32_t id; uint8_t len; uint8_t d[64]; void *to; } trace[MAXF];
static int ntrace, ndelivered;
static uint32_t now_us;
static int bus_cut;            /* frames are recorded but not delivered */

struct node { IsoTpLink link; struct node *peer; uint8_t sbuf[512], rbuf[512]; };
static struct node A, B;

void isotp_user_debug(const char *message, ...) { (void)message; }
uint32_t isotp_user_get_us(void) { return now_us; }
int isotp_user_send_can(const uint32_t id, const uint8_t *data, const uint8_t size, const uint8_t flags, void *arg) {
  struct node *n = arg; (void)flags;
  if (ntrace < MAXF) { trace[ntrace].id = id; trace[ntrace].len = size; memcpy(trace[ntrace].d, data, size); trace[ntrace].to = n->peer; ntrace++; }
  return ISOTP_RET_OK;
}
static void deliver(void) {
  while (ndelivered < ntrace) {
    int i = ndelivered++;
    if (!bus_cut) isotp_on_can_message(&((struct node *)trace[i].to)->link, trace[i].d, trace[i].len);
  }
}
static void setup(void) {
  memset(&A, 0, sizeof A); memset(&B, 0, sizeof B);
  isotp_init_link(&A.link, 0x7E0, A.sbuf, sizeof A.sbuf, A.rbuf, sizeof A.rbuf);
  isotp_init_link(&B.link, 0x7E8, B.sbuf, sizeof B.sbuf, B.rbuf, sizeof B.rbuf);
  A.peer = &B; B.peer = &A; A.link.user_send_can_arg = &A; B.link.user_send_can_arg = &B;
  ntrace = 0; ndelivered = 0; bus_cut = 0;
}
static void spin(int n) { while (n-- > 0) { now_us += 1000; deliver(); isotp_poll(&A.link); isotp_poll(&B.link); } }
static int starts(int i, const char *hex) {       /* trace[i] begins with these bytes */
  if (i >= ntrace) return 0;
  for (int k = 0; hex[2*k]; k++) { unsigned v; sscanf(hex + 2*k, "%2x", &v); if (k >= trace[i].len || trace[i].d[k] != v) return 0; }
  return 1;
}

int main(void) {
  QT_BEGIN("isotp-c");
  uint8_t msg[512], got[512]; uint32_t n = 0;
  for (unsigned i = 0; i < sizeof msg; i++) msg[i] = (uint8_t)(i * 7 + 1);

  setup();
  QT_CHECK(isotp_receive(&B.link, got, sizeof got, &n) == ISOTP_RET_NO_DATA, "isotp_receive on an idle link: ISOTP_RET_NO_DATA");
  static const uint8_t m3[] = { 1, 2, 3 };
  QT_CHECK(isotp_send(&A.link, m3, 3) == ISOTP_RET_OK, "send 3 bytes");
  QT_CHECK(ntrace == 1 && trace[0].id == 0x7E0 && trace[0].len == 8 && starts(0, "03010203AAAAAAAA"), "one Single Frame 03 01 02 03, padded with AA, id 7E0");
  spin(1); n = 0;
  QT_CHECK(isotp_receive(&B.link, got, sizeof got, &n) == ISOTP_RET_OK && n == 3 && memcmp(got, m3, 3) == 0, "the peer receives the 3 bytes");

  setup();
  QT_CHECK(isotp_send(&A.link, msg, 20) == ISOTP_RET_OK, "send 20 bytes");
  QT_CHECK(isotp_send(&A.link, msg, 5) == ISOTP_RET_INPROGRESS, "a second send during the transfer: ISOTP_RET_INPROGRESS");
  spin(20);
  QT_CHECK(ntrace == 4, "four frames on the bus (FF, FC, CF, CF)");
  QT_CHECK(starts(0, "1014") && trace[0].id == 0x7E0 && memcmp(trace[0].d + 2, msg, 6) == 0, "First Frame 10 14 + 6 data bytes");
  QT_CHECK(starts(1, "300800") && trace[1].id == 0x7E8, "Flow Control 30 08 00 from the receiver (id 7E8)");
  QT_CHECK(starts(2, "21") && memcmp(trace[2].d + 1, msg + 6, 7) == 0, "Consecutive Frame 21 + 7 data bytes");
  QT_CHECK(starts(3, "22") && memcmp(trace[3].d + 1, msg + 13, 7) == 0, "Consecutive Frame 22 + the last 7 bytes");
  n = 0;
  QT_CHECK(isotp_receive(&B.link, got, sizeof got, &n) == ISOTP_RET_OK && n == 20 && memcmp(got, msg, 20) == 0, "the peer receives the 20 bytes");
  QT_CHECK(A.link.send_status == ISOTP_SEND_STATUS_IDLE, "the sender is idle again");

  setup();
  QT_CHECK(isotp_send(&A.link, msg, 300) == ISOTP_RET_OK, "send 300 bytes A -> B");
  spin(200);
  n = 0;
  QT_CHECK(isotp_receive(&B.link, got, sizeof got, &n) == ISOTP_RET_OK && n == 300 && memcmp(got, msg, 300) == 0, "B receives the 300 bytes");
  int fc = 0; for (int i = 0; i < ntrace; i++) if (trace[i].id == 0x7E8 && (trace[i].d[0] & 0xF0) == 0x30) fc++;
  printf("  info 300 bytes: %d frames, %d Flow Control\n", ntrace, fc);
  QT_CHECK(ntrace == 1 + 42 + fc && fc == 6, "1 First Frame, 42 Consecutive Frames, 6 Flow Control frames (block size 8)");
  QT_CHECK(isotp_send(&B.link, msg + 100, 300) == ISOTP_RET_OK, "send 300 bytes B -> A");
  spin(200);
  n = 0;
  QT_CHECK(isotp_receive(&A.link, got, sizeof got, &n) == ISOTP_RET_OK && n == 300 && memcmp(got, msg + 100, 300) == 0, "A receives the 300 bytes");

  setup();
  QT_CHECK(isotp_set_tx_dl(&A.link, 64) == ISOTP_RET_OK && isotp_get_tx_dl(&A.link) == 64, "TX_DL 64 (CAN FD)");
  QT_CHECK(isotp_set_tx_dl(&A.link, 9) != ISOTP_RET_OK, "TX_DL 9 is refused");
  QT_CHECK(isotp_send(&A.link, msg, 100) == ISOTP_RET_OK, "send 100 bytes with 64-byte frames");
  spin(20);
  int big = 0; for (int i = 0; i < ntrace; i++) if (trace[i].id == 0x7E0) big++;
  QT_CHECK(big == 2 && trace[0].len == 64 && starts(0, "1064"), "one 64-byte First Frame 10 64 and one Consecutive Frame");
  n = 0;
  QT_CHECK(isotp_receive(&B.link, got, sizeof got, &n) == ISOTP_RET_OK && n == 100 && memcmp(got, msg, 100) == 0, "the peer receives the 100 bytes");

  setup();
  QT_CHECK(isotp_send(&A.link, msg, 513) == ISOTP_RET_OVERFLOW, "a message larger than the send buffer: ISOTP_RET_OVERFLOW");
  bus_cut = 1;
  QT_CHECK(isotp_send(&A.link, msg, 50) == ISOTP_RET_OK, "send 50 bytes on a bus without a receiver");
  spin(50);
  QT_CHECK(A.link.send_status == ISOTP_SEND_STATUS_INPROGRESS, "still waiting for Flow Control after 50 ms");
  spin(100);
  QT_CHECK(A.link.send_status == ISOTP_SEND_STATUS_ERROR && A.link.send_protocol_result == ISOTP_PROTOCOL_RESULT_TIMEOUT_BS,
           "after the timeout: ISOTP_SEND_STATUS_ERROR, ISOTP_PROTOCOL_RESULT_TIMEOUT_BS");
  QT_CHECK(ntrace == 1, "only the First Frame was sent");
  QT_END();
}
