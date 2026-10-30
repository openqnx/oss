#include "common.h"
#include <string.h>
#include <ogg/ogg.h>
#include <opusfile.h>
/* opusfile: an Ogg page carrying an OpusHead packet (stereo, 48 kHz input,
 * pre-skip 312), made here with libogg, recognised by op_test with those
 * values; an OpusTags packet parsed into vendor and comments. */
static void le16(unsigned char *p, unsigned v) { p[0] = v & 255; p[1] = v >> 8; }
static void le32(unsigned char *p, unsigned v) { for (int i = 0; i < 4; i++) p[i] = (v >> (8 * i)) & 255; }
int main(void) {
  QT_BEGIN("opusfile");
  unsigned char head[19]; memcpy(head, "OpusHead", 8); head[8] = 1; head[9] = 2;
  le16(head + 10, 312); le32(head + 12, 48000); le16(head + 16, 0); head[18] = 0;
  ogg_stream_state os; ogg_stream_init(&os, 0x51a7);
  ogg_packet pk; memset(&pk, 0, sizeof pk); pk.packet = head; pk.bytes = sizeof head; pk.b_o_s = 1;
  ogg_stream_packetin(&os, &pk);
  ogg_page pg; QT_CHECK(ogg_stream_flush(&os, &pg) != 0, "Ogg page made");
  unsigned char buf[512]; memcpy(buf, pg.header, pg.header_len); memcpy(buf + pg.header_len, pg.body, pg.body_len);
  OpusHead oh;
  int r = op_test(&oh, buf, pg.header_len + pg.body_len);
  printf("  info op_test=%d channels=%d pre_skip=%u rate=%u\n", r, oh.channel_count, oh.pre_skip, oh.input_sample_rate);
  QT_CHECK(r == 0, "op_test recognises the Opus stream");
  QT_CHECK(oh.channel_count == 2 && oh.pre_skip == 312 && oh.input_sample_rate == 48000, "OpusHead fields");
  ogg_stream_clear(&os);
  unsigned char tags[64]; size_t n = 0;
  memcpy(tags, "OpusTags", 8); n = 8;
  le32(tags + n, 3); n += 4; memcpy(tags + n, "qnx", 3); n += 3;
  le32(tags + n, 1); n += 4; le32(tags + n, 11); n += 4; memcpy(tags + n, "TITLE=Test1", 11); n += 11;
  OpusTags t;
  QT_CHECK(opus_tags_parse(&t, tags, n) == 0, "opus_tags_parse");
  QT_CHECK(strcmp(t.vendor, "qnx") == 0 && t.comments == 1, "vendor and one comment");
  const char *title = opus_tags_query(&t, "TITLE", 0);
  QT_CHECK(title && strcmp(title, "Test1") == 0, "TITLE=Test1");
  opus_tags_clear(&t);
  QT_END();
}
