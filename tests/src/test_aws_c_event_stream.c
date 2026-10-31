/* aws-c-event-stream: the vnd.amazon.eventstream wire format -- the empty
 * message's known bytes and CRCs, a message with typed headers and a payload
 * encoded and then decoded by the streaming decoder fed in small chunks. */
#include "common.h"
#include <aws/common/common.h>
#include <aws/checksums/crc.h>
#include <aws/event-stream/event_stream.h>

struct seen { int preludes, headers, complete, errors; char ctype[64]; int32_t seq; char payload[128]; size_t plen; };
static void on_payload(struct aws_event_stream_streaming_decoder *d, struct aws_byte_buf *p, int8_t fin, void *ud) {
  (void)d; (void)fin;
  struct seen *s = ud;
  if (s->plen + p->len < sizeof s->payload) { memcpy(s->payload + s->plen, p->buffer, p->len); s->plen += p->len; }
}
static void on_prelude(struct aws_event_stream_streaming_decoder *d, struct aws_event_stream_message_prelude *pr, void *ud) {
  (void)d; (void)pr; ((struct seen *)ud)->preludes++;
}
static void on_header(struct aws_event_stream_streaming_decoder *d, struct aws_event_stream_message_prelude *pr,
                      struct aws_event_stream_header_value_pair *h, void *ud) {
  (void)d; (void)pr;
  struct seen *s = ud;
  s->headers++;
  struct aws_byte_buf n = aws_event_stream_header_name(h);
  if (n.len == 13 && memcmp(n.buffer, ":content-type", 13) == 0) {
    struct aws_byte_buf v = aws_event_stream_header_value_as_string(h);
    snprintf(s->ctype, sizeof s->ctype, "%.*s", (int)v.len, (const char *)v.buffer);
  }
  if (n.len == 3 && memcmp(n.buffer, "seq", 3) == 0) s->seq = aws_event_stream_header_value_as_int32(h);
}
static void on_complete(struct aws_event_stream_streaming_decoder *d, uint32_t crc, void *ud) {
  (void)d; (void)crc; ((struct seen *)ud)->complete++;
}
static void on_error(struct aws_event_stream_streaming_decoder *d, struct aws_event_stream_message_prelude *pr, int code,
                     const char *msg, void *ud) {
  (void)d; (void)pr;
  printf("  info decoder error %d: %s\n", code, msg ? msg : "(null)");
  ((struct seen *)ud)->errors++;
}

int main(void) {
  QT_BEGIN("aws-c-event-stream");
  struct aws_allocator *al = aws_default_allocator();
  aws_event_stream_library_init(al);

  struct aws_event_stream_message empty;
  QT_CHECK(aws_event_stream_message_init(&empty, al, NULL, NULL) == AWS_OP_SUCCESS, "encode an empty message");
  static const uint8_t want[16] = {0, 0, 0, 0x10, 0, 0, 0, 0, 0x05, 0xc2, 0x48, 0xeb, 0x7d, 0x98, 0xc8, 0xff};
  QT_CHECK(aws_event_stream_message_total_length(&empty) == 16 &&
           memcmp(aws_event_stream_message_buffer(&empty), want, 16) == 0, "empty message bytes (with both CRCs) known");
  QT_CHECK(aws_event_stream_message_prelude_crc(&empty) == 0x05c248eb && aws_event_stream_message_message_crc(&empty) == 0x7d98c8ff,
           "prelude and message CRC accessors");
  aws_event_stream_message_clean_up(&empty);

  struct aws_array_list headers;
  aws_event_stream_headers_list_init(&headers, al);
  aws_event_stream_add_string_header(&headers, ":content-type", 13, "application/json", 16, 1);
  aws_event_stream_add_string_header(&headers, ":event-type", 11, "Records", 7, 1);
  aws_event_stream_add_int32_header(&headers, "seq", 3, -123456);
  aws_event_stream_add_bool_header(&headers, "last", 4, 1);
  const char *body = "{\"records\":[1,2,3],\"from\":\"qnx\"}";
  struct aws_byte_buf pay = aws_byte_buf_from_c_str(body);
  struct aws_event_stream_message msg;
  QT_CHECK(aws_event_stream_message_init(&msg, al, &headers, &pay) == AWS_OP_SUCCESS, "encode a message with 4 headers");
  uint32_t total = aws_event_stream_message_total_length(&msg);
  const uint8_t *buf = aws_event_stream_message_buffer(&msg);
  printf("  info encoded %u bytes, headers %u\n", total, aws_event_stream_message_headers_len(&msg));
  QT_CHECK(aws_event_stream_message_payload_len(&msg) == strlen(body), "payload length");
  QT_CHECK(aws_checksums_crc32(buf, (int)total - 4, 0) == aws_event_stream_message_message_crc(&msg),
           "trailing message CRC is CRC32 of the rest");

  struct aws_event_stream_streaming_decoder dec;
  struct seen s; memset(&s, 0, sizeof s);
  struct aws_event_stream_streaming_decoder_options o = {0};
  o.on_payload_segment = on_payload; o.on_prelude = on_prelude; o.on_header = on_header;
  o.on_complete = on_complete; o.on_error = on_error; o.user_data = &s;
  aws_event_stream_streaming_decoder_init_from_options(&dec, al, &o);
  /* Two copies of the message back to back, fed 7 bytes at a time. */
  int rc = AWS_OP_SUCCESS;
  for (int copy = 0; copy < 2; ++copy)
    for (uint32_t off = 0; off < total && rc == AWS_OP_SUCCESS; off += 7) {
      struct aws_byte_buf chunk = aws_byte_buf_from_array(buf + off, total - off < 7 ? total - off : 7);
      rc = aws_event_stream_streaming_decoder_pump(&dec, &chunk);
    }
  QT_CHECK(rc == AWS_OP_SUCCESS && s.errors == 0, "streaming decoder accepts both messages");
  QT_CHECK(s.preludes == 2 && s.complete == 2 && s.headers == 8, "2 preludes, 8 headers, 2 completions");
  QT_CHECK(strcmp(s.ctype, "application/json") == 0 && s.seq == -123456, "string and int32 header values");
  QT_CHECK(s.plen == 2 * strlen(body) && memcmp(s.payload, body, strlen(body)) == 0, "payload reassembled");
  aws_event_stream_streaming_decoder_clean_up(&dec);

  /* Corrupt one payload byte: the message CRC check must fail. */
  uint8_t *bad = aws_mem_acquire(al, total);
  memcpy(bad, buf, total);
  bad[total - 6] ^= 0x40;
  memset(&s, 0, sizeof s);
  aws_event_stream_streaming_decoder_init_from_options(&dec, al, &o);
  struct aws_byte_buf bb = aws_byte_buf_from_array(bad, total);
  rc = aws_event_stream_streaming_decoder_pump(&dec, &bb);
  QT_CHECK(rc != AWS_OP_SUCCESS || s.errors > 0, "corrupted message is rejected (CRC mismatch)");
  QT_CHECK(s.complete == 0, "corrupted message does not complete");
  aws_event_stream_streaming_decoder_clean_up(&dec);
  aws_mem_release(al, bad);

  aws_event_stream_message_clean_up(&msg);
  aws_event_stream_headers_list_cleanup(&headers);
  aws_event_stream_library_clean_up();
  QT_END();
}
