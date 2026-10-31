/* libogg: the Ogg container's bit packer (LSb and MSb), packets through a
 * logical stream into pages, the sync layer re-reading those pages fed in
 * odd-sized chunks, CRC rejection of a corrupted page, and granule
 * positions/EOS flags carried through. */
#include "common.h"
#include <stdlib.h>
#include <ogg/ogg.h>

#define NPKT 50
static int pkt_len(int i) { return 1 + (i * 97) % 700; }
static unsigned char pkt_byte(int i, int j) { return (unsigned char)(i * 31 + j * 7); }

int main(void) {
  QT_BEGIN("libogg");
  oggpack_buffer pb;
  oggpack_writeinit(&pb);
  oggpack_write(&pb, 5, 3); oggpack_write(&pb, 0x1234, 16); oggpack_write(&pb, 1, 1); oggpack_write(&pb, 0xABCDEF, 24);
  long bytes = oggpack_bytes(&pb);
  unsigned char *wb = oggpack_get_buffer(&pb);
  QT_CHECK(bytes == 6 && oggpack_bits(&pb) == 44, "packed 44 bits into 6 bytes");
  QT_CHECK(wb[0] == 0xA5 && wb[1] == 0x91, "LSb-first bit order (first bytes a5 91)");
  oggpack_buffer rb;
  oggpack_readinit(&rb, wb, bytes);
  QT_CHECK(oggpack_read(&rb, 3) == 5 && oggpack_read(&rb, 16) == 0x1234 && oggpack_read(&rb, 1) == 1 &&
           oggpack_read(&rb, 24) == 0xABCDEF, "bit reader returns the values");
  oggpack_writeclear(&pb);
  oggpackB_writeinit(&pb);
  oggpackB_write(&pb, 0xA, 4); oggpackB_write(&pb, 0x5, 4);
  QT_CHECK(oggpackB_get_buffer(&pb)[0] == 0xA5, "MSb-first packer");
  oggpackB_writeclear(&pb);

  ogg_stream_state os, is;
  ogg_stream_init(&os, 0x51A2B3C4);
  ogg_stream_init(&is, 0x51A2B3C4);
  unsigned char *file = NULL; size_t flen = 0;
  int pages = 0;
  for (int i = 0; i < NPKT; ++i) {
    unsigned char buf[800];
    for (int j = 0; j < pkt_len(i); ++j) buf[j] = pkt_byte(i, j);
    ogg_packet op = {buf, pkt_len(i), i == 0, i == NPKT - 1, (ogg_int64_t)(i + 1) * 1000, i};
    ogg_stream_packetin(&os, &op);
    ogg_page og;
    while (i == NPKT - 1 ? ogg_stream_flush(&os, &og) : ogg_stream_pageout(&os, &og)) {
      file = realloc(file, flen + og.header_len + og.body_len);
      memcpy(file + flen, og.header, og.header_len); memcpy(file + flen + og.header_len, og.body, og.body_len);
      flen += og.header_len + og.body_len;
      ++pages;
    }
  }
  printf("  info %d packets -> %d pages, %zu bytes\n", NPKT, pages, flen);
  QT_CHECK(pages >= 3 && file && memcmp(file, "OggS", 4) == 0, "pages written, capture pattern OggS");

  ogg_sync_state oy; ogg_sync_init(&oy);
  int got = 0, ok = 1, bos = 0, eos = 0; ogg_int64_t last_gp = -1;
  size_t off = 0, chunk = 37;
  while (off < flen) {
    size_t n = flen - off < chunk ? flen - off : chunk;
    memcpy(ogg_sync_buffer(&oy, (long)n), file + off, n);
    ogg_sync_wrote(&oy, (long)n);
    off += n; chunk = chunk * 3 % 1000 + 1;
    ogg_page og;
    while (ogg_sync_pageout(&oy, &og) == 1) {
      if (ogg_page_serialno(&og) != 0x51A2B3C4) ok = 0;
      if (ogg_page_bos(&og)) bos++;
      if (ogg_page_eos(&og)) eos++;
      ogg_stream_pagein(&is, &og);
      ogg_packet op;
      while (ogg_stream_packetout(&is, &op) == 1) {
        if (op.bytes != pkt_len(got)) ok = 0;
        else for (int j = 0; j < op.bytes; ++j) if (op.packet[j] != pkt_byte(got, j)) { ok = 0; break; }
        if (op.granulepos != -1) last_gp = op.granulepos;
        ++got;
      }
    }
  }
  QT_CHECK(got == NPKT && ok, "all packets re-read intact through the sync layer");
  QT_CHECK(bos == 1 && eos == 1 && last_gp == NPKT * 1000, "BOS/EOS flags and final granule position");

  /* Flip a byte inside the last page's body: that page's CRC must fail and
     every other page must still come out. */
  ogg_sync_reset(&oy);
  file[flen - 3] ^= 0xFF;
  memcpy(ogg_sync_buffer(&oy, (long)flen), file, flen);
  ogg_sync_wrote(&oy, (long)flen);
  ogg_page og;
  int r, good = 0, holes = 0;
  while ((r = ogg_sync_pageout(&oy, &og)) != 0) { if (r == 1) ++good; else ++holes; }
  printf("  info after corruption: %d good page(s), %d hole(s)\n", good, holes);
  QT_CHECK(holes >= 1, "corrupted page detected (CRC mismatch -> hole)");
  QT_CHECK(good == pages - 1, "all other pages still verified");
  free(file);
  ogg_sync_clear(&oy); ogg_stream_clear(&os); ogg_stream_clear(&is);
  QT_END();
}
