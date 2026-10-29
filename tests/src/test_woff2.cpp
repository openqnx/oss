/* woff2: the decompressor's error path and, when a font is available on the
 * target, a real WOFF2 compress/decompress round trip. */
#include "common.h"
#include <woff2/decode.h>
#include <woff2/encode.h>
#include <string>
#include <vector>
#include <cstdio>
static const char *candidates[] = {
  "/usr/local/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/DejaVuSans.ttf",
  "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
  nullptr
};
int main() {
  QT_BEGIN("woff2");
  /* Garbage must be rejected rather than crash or read out of bounds. */
  const uint8_t junk[] = { 'n','o','t','a','w','o','f','f','2',0,0,0 };
  QT_CHECK(woff2::ComputeWOFF2FinalSize(junk, sizeof junk) == 0 ||
           true, "ComputeWOFF2FinalSize on garbage returns without crashing");
  std::string out;
  woff2::WOFF2StringOut sink(&out);
  QT_CHECK(!woff2::ConvertWOFF2ToTTF(junk, sizeof junk, &sink),
           "garbage is rejected by ConvertWOFF2ToTTF");

  const char *path = nullptr;
  std::vector<uint8_t> ttf;
  for (int i = 0; candidates[i] && !path; ++i) {
    if (FILE *f = fopen(candidates[i], "rb")) {
      fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
      ttf.resize((size_t)n);
      if (fread(ttf.data(), 1, (size_t)n, f) == (size_t)n) path = candidates[i];
      fclose(f);
    }
  }
  if (!path) {
    printf("  info no TTF on this target; checked the error path only\n");
    QT_END();
  }
  printf("  info using %s (%zu bytes)\n", path, ttf.size());
  size_t cap = woff2::MaxWOFF2CompressedSize(ttf.data(), ttf.size());
  std::string woff2_out(cap, '\0');
  size_t len = cap;
  QT_CHECK(woff2::ConvertTTFToWOFF2(ttf.data(), ttf.size(),
                                    (uint8_t *)woff2_out.data(), &len),
           "ConvertTTFToWOFF2");
  QT_CHECK(len > 0 && len < ttf.size(), "WOFF2 is smaller than the TTF (brotli works)");
  woff2_out.resize(len);
  std::string ttf_back;
  woff2::WOFF2StringOut back(&ttf_back);
  QT_CHECK(woff2::ConvertWOFF2ToTTF((const uint8_t *)woff2_out.data(), len, &back),
           "ConvertWOFF2ToTTF");
  QT_CHECK(ttf_back.size() > 0, "a TTF came back out");
  QT_END();
}
