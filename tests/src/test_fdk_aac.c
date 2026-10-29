/* fdk-aac: encode PCM to AAC-LC and decode it back. */
#include "common.h"
#include <stdlib.h>
#include <math.h>
#include <fdk-aac/aacenc_lib.h>
#include <fdk-aac/aacdecoder_lib.h>
#define RATE 44100
#define FRAME 1024
int main(void) {
  QT_BEGIN("fdk-aac");
  HANDLE_AACENCODER enc = NULL;
  QT_CHECK(aacEncOpen(&enc, 0, 1) == AACENC_OK, "aacEncOpen");
  if (!enc) QT_END();
  QT_CHECK(aacEncoder_SetParam(enc, AACENC_AOT, AOT_AAC_LC) == AACENC_OK, "set AOT");
  aacEncoder_SetParam(enc, AACENC_SAMPLERATE, RATE);
  aacEncoder_SetParam(enc, AACENC_CHANNELMODE, MODE_1);
  aacEncoder_SetParam(enc, AACENC_BITRATE, 64000);
  aacEncoder_SetParam(enc, AACENC_TRANSMUX, 2);   /* ADTS */
  QT_CHECK(aacEncEncode(enc, NULL, NULL, NULL, NULL) == AACENC_OK, "encoder init pass");
  AACENC_InfoStruct info;
  QT_CHECK(aacEncInfo(enc, &info) == AACENC_OK, "aacEncInfo");
  printf("  info frame length %u, delay %u\n", info.frameLength, info.nDelay);

  HANDLE_AACDECODER dec = aacDecoder_Open(TT_MP4_ADTS, 1);
  QT_CHECK(dec != NULL, "aacDecoder_Open");

  INT_PCM pcm[FRAME];
  unsigned char out[4096];
  int packets = 0, decoded_frames = 0;
  for (int f = 0; f < 8; ++f) {
    for (int i = 0; i < FRAME; ++i)
      pcm[i] = (INT_PCM)(8000.0 * sin(2.0 * M_PI * 440.0 * (f * FRAME + i) / RATE));
    void *in_bufs[1] = { pcm };
    INT in_ids[1] = { IN_AUDIO_DATA }, in_sizes[1] = { FRAME * (INT)sizeof(INT_PCM) };
    INT in_el[1] = { (INT)sizeof(INT_PCM) };
    AACENC_BufDesc ibuf = { 1, in_bufs, in_ids, in_sizes, in_el };
    void *out_bufs[1] = { out };
    INT out_ids[1] = { OUT_BITSTREAM_DATA }, out_sizes[1] = { (INT)sizeof out };
    INT out_el[1] = { 1 };
    AACENC_BufDesc obuf = { 1, out_bufs, out_ids, out_sizes, out_el };
    AACENC_InArgs iargs = { FRAME, 0 };
    AACENC_OutArgs oargs;
    memset(&oargs, 0, sizeof oargs);
    if (aacEncEncode(enc, &ibuf, &obuf, &iargs, &oargs) != AACENC_OK) continue;
    if (oargs.numOutBytes <= 0) continue;
    ++packets;
    UCHAR *p = out;
    UINT valid = (UINT)oargs.numOutBytes, size = valid;
    if (aacDecoder_Fill(dec, &p, &size, &valid) != AAC_DEC_OK) continue;
    INT_PCM back[FRAME * 2];
    if (aacDecoder_DecodeFrame(dec, back, FRAME * 2, 0) == AAC_DEC_OK) ++decoded_frames;
  }
  QT_CHECK(packets >= 4, "ADTS packets produced");
  QT_CHECK(decoded_frames >= 1, "at least one frame decoded back");
  printf("  info %d packet(s), %d decoded frame(s)\n", packets, decoded_frames);
  CStreamInfo *si = aacDecoder_GetStreamInfo(dec);
  QT_CHECK(si && si->sampleRate == RATE, "decoded stream reports 44100 Hz");
  aacDecoder_Close(dec);
  aacEncClose(&enc);
  QT_END();
}
