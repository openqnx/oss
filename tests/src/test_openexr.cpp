// openexr: write a known 64x32 RGBA half-float image -- a gradient with values
// above 1.0 (HDR) and a custom header attribute -- to an EXR file with each
// lossless compression (none, RLE, ZIPS, ZIP, PIZ, HTJ2K) and read it back:
// size, data window, attribute and every pixel must be identical. Plus a
// lossy B44 file read back within half precision of the original, a tiled
// file, and the C core library (exr_start_read) reading the same file.
#include "common.h"
#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfTiledRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfStringAttribute.h>
#include <OpenEXR/ImfCompression.h>
#include <OpenEXR/openexr.h>
#include <Imath/half.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <unistd.h>

using namespace OPENEXR_IMF_NAMESPACE;
static const int W = 64, H = 32;

static Rgba ref(int x, int y) {
  return Rgba(half(x / 8.0f), half(y / 4.0f), half((x + y) / 96.0f), half(x == y ? 0.5f : 1.0f));
}

static bool write_read(const std::string& fn, Compression c, bool exact) {
  {
    Header hdr(W, H);
    hdr.compression() = c;
    hdr.insert("qnxport", StringAttribute("openexr known image"));
    RgbaOutputFile out(fn.c_str(), hdr, WRITE_RGBA);
    Array2D<Rgba> px(H, W);
    for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) px[y][x] = ref(x, y);
    out.setFrameBuffer(&px[0][0], 1, W);
    out.writePixels(H);
  }
  RgbaInputFile in(fn.c_str());
  Imath::Box2i dw = in.dataWindow();
  if (dw.max.x - dw.min.x + 1 != W || dw.max.y - dw.min.y + 1 != H) return false;
  if (in.header().compression() != c) return false;
  const StringAttribute* a = in.header().findTypedAttribute<StringAttribute>("qnxport");
  if (!a || a->value() != "openexr known image") return false;
  Array2D<Rgba> px(H, W);
  in.setFrameBuffer(&px[0][0], 1, W);
  in.readPixels(dw.min.y, dw.max.y);
  for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) {
    Rgba r = ref(x, y), p = px[y][x];
    if (exact) {
      if (p.r.bits() != r.r.bits() || p.g.bits() != r.g.bits() || p.b.bits() != r.b.bits() || p.a.bits() != r.a.bits()) return false;
    } else if (std::fabs(float(p.r) - float(r.r)) > 0.05f * std::fabs(float(r.r)) + 0.01f) return false;
  }
  return true;
}

int main() {
  QT_BEGIN("openexr");
  char dir[] = "/tmp/qnxport-openexr.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "temp dir"); QT_END(); }
  std::string d(dir);
  struct { Compression c; const char* n; } cs[] = {
    { NO_COMPRESSION, "none" }, { RLE_COMPRESSION, "RLE" }, { ZIPS_COMPRESSION, "ZIPS" },
    { ZIP_COMPRESSION, "ZIP" }, { PIZ_COMPRESSION, "PIZ" }, { HTJ2K256_COMPRESSION, "HTJ2K (OpenJPH)" } };
  for (auto& c : cs) {
    bool ok = false;
    try { ok = write_read(d + "/img.exr", c.c, true); } catch (const std::exception& e) { printf("  info %s: %s\n", c.n, e.what()); }
    QT_CHECK(ok, (std::string("lossless round trip, compression ") + c.n).c_str());
  }
  bool ok = false;
  try { ok = write_read(d + "/b44.exr", B44_COMPRESSION, false); } catch (const std::exception& e) { printf("  info B44: %s\n", e.what()); }
  QT_CHECK(ok, "lossy B44 round trip within tolerance");

  // tiled: 16x16 tiles, one level
  try {
    {
      Header hdr(W, H);
      TiledRgbaOutputFile out((d + "/tiled.exr").c_str(), hdr, WRITE_RGBA, 16, 16, ONE_LEVEL);
      Array2D<Rgba> px(H, W);
      for (int y = 0; y < H; y++) for (int x = 0; x < W; x++) px[y][x] = ref(x, y);
      out.setFrameBuffer(&px[0][0], 1, W);
      out.writeTiles(0, out.numXTiles() - 1, 0, out.numYTiles() - 1);
    }
    TiledRgbaInputFile in((d + "/tiled.exr").c_str());
    QT_CHECK(in.numXTiles() == 4 && in.numYTiles() == 2, "tiled file has 4x2 tiles of 16x16");
    Array2D<Rgba> px(H, W);
    in.setFrameBuffer(&px[0][0], 1, W);
    in.readTiles(0, in.numXTiles() - 1, 0, in.numYTiles() - 1);
    QT_CHECK(px[31][63].r.bits() == ref(63, 31).r.bits() && px[5][7].g.bits() == ref(7, 5).g.bits(), "tiled pixels read back");
  } catch (const std::exception& e) { printf("  info tiled: %s\n", e.what()); QT_CHECK(0, "tiled file"); }

  // the C core library on the ZIP-less file written last (HTJ2K img.exr was
  // overwritten by each round; write a ZIP one for it)
  write_read(d + "/core.exr", ZIP_COMPRESSION, true);
  exr_context_t ctx = nullptr;
  exr_context_initializer_t init = EXR_DEFAULT_CONTEXT_INITIALIZER;
  exr_result_t rv = exr_start_read(&ctx, (d + "/core.exr").c_str(), &init);
  QT_CHECK(rv == EXR_ERR_SUCCESS, "exr_start_read opens the file (C core)");
  if (rv == EXR_ERR_SUCCESS) {
    exr_attr_box2i_t dw;
    exr_compression_t comp;
    int np = 0;
    exr_get_count(ctx, &np);
    exr_get_data_window(ctx, 0, &dw);
    exr_get_compression(ctx, 0, &comp);
    QT_CHECK(np == 1 && dw.max.x == W - 1 && dw.max.y == H - 1 && comp == EXR_COMPRESSION_ZIP, "core: one part, 64x32, ZIP");
    const exr_attr_chlist_t* ch = nullptr;
    exr_get_channels(ctx, 0, &ch);
    QT_CHECK(ch && ch->num_channels == 4 && std::string(ch->entries[0].name.str) == "A", "core: channels A,B,G,R");
    exr_finish(&ctx);
  }
  std::string cmd = "rm -rf " + d; if (system(cmd.c_str())) {}
  QT_END();
}
