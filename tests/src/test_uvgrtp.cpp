// uvgRTP: RTP over UDP loopback between a sending and a receiving media
// stream in this process, with known answers:
//  - generic payload (PCMU payload type 0): three 160-byte frames with given
//    timestamps arrive intact, with the sender's SSRC (set with RCC_SSRC),
//    consecutive sequence numbers and the timestamps given;
//  - a 9000-byte generic frame with RCE_FRAGMENT_GENERIC: fragmented to the
//    MTU on the wire, reassembled to the same 9000 bytes;
//  - an H.265 access unit (VPS, SPS, PPS and a 6000-byte IDR slice behind
//    Annex B start codes): sent as single NAL units and fragmentation units
//    (RFC 7798), received as the same NAL units with start codes;
//  - the version reported is 4.0.1.
#include "common.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>
#include <uvgrtp/lib.hh>

static const uint16_t PA = 42400, PB = 42402, PC = 42404, PD = 42406, PE = 42408, PF = 42410;

static std::vector<uint8_t> pattern(size_t n, unsigned seed) {
  std::vector<uint8_t> v(n);
  uint32_t x = seed * 2654435761u + 1;
  for (size_t i = 0; i < n; i++) { x = x * 1103515245u + 12345u; v[i] = (uint8_t)(x >> 16); }
  return v;
}

int main() {
  QT_BEGIN("uvgrtp");
  printf("  info uvgRTP %s\n", uvgrtp::get_version_cstr());
  QT_CHECK(uvgrtp::get_version_major() == 4 && uvgrtp::get_version_minor() == 0 &&
           uvgrtp::get_version_patch() == 1, "version 4.0.1");

  uvgrtp::context ctx;
  uvgrtp::session *sess = ctx.create_session("127.0.0.1", "127.0.0.1");
  QT_CHECK(sess != nullptr, "session on 127.0.0.1");
  if (!sess) QT_END();

  // ---- generic, one packet per frame ----
  uvgrtp::media_stream *tx = sess->create_stream(PA, PB, RTP_FORMAT_GENERIC, RCE_SEND_ONLY);
  uvgrtp::media_stream *rx = sess->create_stream(PB, PA, RTP_FORMAT_GENERIC, RCE_RECEIVE_ONLY);
  QT_CHECK(tx && rx, "generic send and receive streams");
  if (tx && rx) {
    QT_CHECK(tx->configure_ctx(RCC_SSRC, 0x51A7E5) == RTP_OK, "RCC_SSRC set on the sender");
    int got = 0, intact = 0, seq_ok = 1, ts_ok = 1, ssrc_ok = 1, pt_ok = 1;
    int prev_seq = -1;
    for (int k = 0; k < 3; k++) {
      std::vector<uint8_t> f = pattern(160, (unsigned)k);
      if (tx->push_frame(f.data(), f.size(), 8000u * (unsigned)(k + 1), RTP_NO_FLAGS) != RTP_OK)
        printf("  info push_frame %d failed\n", k);
      uvgrtp::frame::rtp_frame *r = rx->pull_frame(2000);
      if (!r) continue;
      got++;
      if (r->payload_len == f.size() && !memcmp(r->payload, f.data(), f.size())) intact++;
      if (prev_seq >= 0 && r->header.seq != (uint16_t)(prev_seq + 1)) seq_ok = 0;
      prev_seq = r->header.seq;
      if (r->header.timestamp != 8000u * (unsigned)(k + 1)) ts_ok = 0;
      if (r->header.ssrc != 0x51A7E5) { ssrc_ok = 0; printf("  info ssrc 0x%x\n", r->header.ssrc); }
      if (r->header.payload != 0 || r->header.version != 2) pt_ok = 0;
      uvgrtp::frame::dealloc_frame(r);
    }
    QT_CHECK(got == 3, "3 generic frames received");
    QT_CHECK(intact == 3, "payloads intact (160 bytes each)");
    QT_CHECK(seq_ok && got == 3, "consecutive sequence numbers");
    QT_CHECK(ts_ok && got == 3, "timestamps 8000, 16000, 24000 as pushed");
    QT_CHECK(ssrc_ok && got == 3, "SSRC 0x0051a7e5 on every packet");
    QT_CHECK(pt_ok && got == 3, "RTP version 2, payload type 0 (PCMU/generic)");
    sess->destroy_stream(tx);
    sess->destroy_stream(rx);
  }

  // ---- generic, fragmented ----
  tx = sess->create_stream(PC, PD, RTP_FORMAT_GENERIC, RCE_SEND_ONLY | RCE_FRAGMENT_GENERIC);
  rx = sess->create_stream(PD, PC, RTP_FORMAT_GENERIC, RCE_RECEIVE_ONLY | RCE_FRAGMENT_GENERIC);
  if (tx && rx) {
    std::vector<uint8_t> f = pattern(9000, 7);
    rtp_error_t e = tx->push_frame(f.data(), f.size(), RTP_NO_FLAGS);
    uvgrtp::frame::rtp_frame *r = rx->pull_frame(3000);
    QT_CHECK(e == RTP_OK && r && r->payload_len == 9000 && !memcmp(r->payload, f.data(), 9000),
             "9000-byte generic frame fragmented and reassembled intact");
    if (r) uvgrtp::frame::dealloc_frame(r);
    sess->destroy_stream(tx);
    sess->destroy_stream(rx);
  } else QT_CHECK(0, "fragmenting generic streams");

  // ---- H.265 ----
  tx = sess->create_stream(PE, PF, RTP_FORMAT_H265, RCE_SEND_ONLY);
  rx = sess->create_stream(PF, PE, RTP_FORMAT_H265, RCE_RECEIVE_ONLY);
  if (tx && rx) {
    // NAL unit headers: type << 1, layer 0, tid 1.
    const uint8_t types[4] = { 32, 33, 34, 19 };   // VPS, SPS, PPS, IDR_W_RADL
    const size_t sizes[4] = { 24, 40, 8, 6000 };
    std::vector<uint8_t> au;
    std::vector<std::vector<uint8_t>> nals;
    for (int i = 0; i < 4; i++) {
      std::vector<uint8_t> n = pattern(sizes[i], 100u + (unsigned)i);
      n[0] = (uint8_t)(types[i] << 1); n[1] = 1;
      for (size_t j = 2; j + 2 < n.size(); j++)   // no emulated start codes in the body
        if (n[j] == 0 && n[j + 1] == 0) n[j + 1] = 0x55;
      nals.push_back(n);
      const uint8_t sc[4] = { 0, 0, 0, 1 };
      au.insert(au.end(), sc, sc + 4);
      au.insert(au.end(), n.begin(), n.end());
    }
    rtp_error_t e = tx->push_frame(au.data(), au.size(), 90000, RTP_NO_FLAGS);
    QT_CHECK(e == RTP_OK, "H.265 access unit pushed (4 NAL units, 6000-byte slice fragmented)");
    int matched = 0, frames = 0;
    for (int i = 0; i < 8 && matched < 4; i++) {
      uvgrtp::frame::rtp_frame *r = rx->pull_frame(2000);
      if (!r) break;
      frames++;
      // Each received frame is one NAL unit, behind a start code.
      const uint8_t *p = r->payload; size_t n = r->payload_len, skip = 0;
      if (n >= 4 && !p[0] && !p[1] && !p[2] && p[3] == 1) skip = 4;
      else if (n >= 3 && !p[0] && !p[1] && p[2] == 1) skip = 3;
      if (matched < 4 && n - skip == nals[matched].size() && !memcmp(p + skip, nals[matched].data(), n - skip))
        matched++;
      else printf("  info H.265 frame %d: %zu bytes, NAL type %d\n", frames, n, n > skip ? (p[skip] >> 1) & 63 : -1);
      uvgrtp::frame::dealloc_frame(r);
    }
    QT_CHECK(matched == 4, "VPS, SPS, PPS and the 6000-byte IDR slice received intact, in order");
    sess->destroy_stream(tx);
    sess->destroy_stream(rx);
  } else QT_CHECK(0, "H.265 streams");

  ctx.destroy_session(sess);
  QT_END();
}
