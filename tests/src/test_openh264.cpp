/* OpenH264: encode frames and decode them back with the matching decoder. */
#include "common.h"
#include <stdlib.h>
#include <wels/codec_api.h>
#define W 176
#define H 144
int main(void) {
  QT_BEGIN("openh264");
  OpenH264Version v = WelsGetCodecVersion();
  printf("  info OpenH264 %u.%u.%u\n", v.uMajor, v.uMinor, v.uRevision);
  ISVCEncoder *enc = NULL;
  QT_CHECK(WelsCreateSVCEncoder(&enc) == 0 && enc, "WelsCreateSVCEncoder");
  if (!enc) QT_END();
  SEncParamBase p;
  memset(&p, 0, sizeof p);
  p.iUsageType = CAMERA_VIDEO_REAL_TIME;
  p.iPicWidth = W; p.iPicHeight = H;
  p.iTargetBitrate = 300000;
  p.fMaxFrameRate = 25.0f;
  QT_CHECK(enc->Initialize(&p) == 0, "encoder Initialize");
  int level = WELS_LOG_QUIET;
  enc->SetOption(ENCODER_OPTION_TRACE_LEVEL, &level);

  ISVCDecoder *dec = NULL;
  QT_CHECK(WelsCreateDecoder(&dec) == 0 && dec, "WelsCreateDecoder");
  SDecodingParam dp;
  memset(&dp, 0, sizeof dp);
  dp.eEcActiveIdc = ERROR_CON_DISABLE;
  dp.sVideoProperty.eVideoBsType = VIDEO_BITSTREAM_AVC;
  QT_CHECK(dec->Initialize(&dp) == 0, "decoder Initialize");
  dec->SetOption(DECODER_OPTION_TRACE_LEVEL, &level);

  unsigned char *buf = (unsigned char *)malloc(W * H * 3 / 2);
  SSourcePicture pic;
  memset(&pic, 0, sizeof pic);
  pic.iPicWidth = W; pic.iPicHeight = H; pic.iColorFormat = videoFormatI420;
  pic.iStride[0] = W; pic.iStride[1] = pic.iStride[2] = W / 2;
  pic.pData[0] = buf; pic.pData[1] = buf + W * H; pic.pData[2] = buf + W * H * 5 / 4;

  int encoded = 0, decoded = 0;
  for (int f = 0; f < 5; ++f) {
    memset(buf, 16 + f * 30, (size_t)(W * H));
    memset(buf + W * H, 128, (size_t)(W * H / 2));
    SFrameBSInfo bs;
    memset(&bs, 0, sizeof bs);
    if (enc->EncodeFrame(&pic, &bs) == cmResultSuccess && bs.eFrameType != videoFrameTypeSkip) {
      ++encoded;
      int total = 0;
      for (int li = 0; li < bs.iLayerNum; ++li) {
        SLayerBSInfo *layer = &bs.sLayerInfo[li];
        for (int n = 0; n < layer->iNalCount; ++n) total += layer->pNalLengthInByte[n];
      }
      unsigned char *out[3] = {0};
      SBufferInfo bi;
      memset(&bi, 0, sizeof bi);
      if (dec->DecodeFrameNoDelay(bs.sLayerInfo[0].pBsBuf, total, out, &bi) == 0 &&
          bi.iBufferStatus == 1)
        ++decoded;
    }
  }
  QT_CHECK(encoded >= 5, "frames encoded");
  QT_CHECK(decoded >= 1, "at least one frame decoded back");
  printf("  info encoded %d, decoded %d\n", encoded, decoded);
  enc->Uninitialize(); WelsDestroySVCEncoder(enc);
  dec->Uninitialize(); WelsDestroyDecoder(dec);
  free(buf);
  QT_END();
}
