/* Exiv2: write EXIF, IPTC and XMP metadata into an in-memory JPEG and read it
 * back through the C++ API. */
#include "common.h"
#include <exiv2/exiv2.hpp>
#include <vector>
#include <string>
/* The smallest valid JPEG: SOI, a quantisation-free grey 1x1, EOI. Exiv2 only
 * needs a parseable container to attach metadata to. */
static const unsigned char kJpeg[] = {
  0xFF,0xD8,0xFF,0xDB,0x00,0x43,0x00, 0x08,0x06,0x06,0x07,0x06,0x05,0x08,0x07,
  0x07,0x07,0x09,0x09,0x08,0x0A,0x0C,0x14,0x0D,0x0C,0x0B,0x0B,0x0C,0x19,0x12,
  0x13,0x0F,0x14,0x1D,0x1A,0x1F,0x1E,0x1D,0x1A,0x1C,0x1C,0x20,0x24,0x2E,0x27,
  0x20,0x22,0x2C,0x23,0x1C,0x1C,0x28,0x37,0x29,0x2C,0x30,0x31,0x34,0x34,0x34,
  0x1F,0x27,0x39,0x3D,0x38,0x32,0x3C,0x2E,0x33,0x34,0x32,
  0xFF,0xC0,0x00,0x0B,0x08,0x00,0x01,0x00,0x01,0x01,0x01,0x11,0x00,
  0xFF,0xC4,0x00,0x1F,0x00,0x00,0x01,0x05,0x01,0x01,0x01,0x01,0x01,0x01,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
  0x09,0x0A,0x0B,
  0xFF,0xDA,0x00,0x08,0x01,0x01,0x00,0x00,0x3F,0x00,0x37,0xFF,0xD9
};
int main() {
  QT_BEGIN("exiv2");
  printf("  info exiv2 %s\n", Exiv2::versionString().c_str());
  Exiv2::XmpParser::initialize();
  try {
    auto img = Exiv2::ImageFactory::open(kJpeg, sizeof kJpeg);
    QT_CHECK(img.get() != nullptr, "ImageFactory::open on an in-memory JPEG");
    img->readMetadata();
    QT_CHECK(img->mimeType() == "image/jpeg", "mime type is image/jpeg");

    img->exifData()["Exif.Image.Make"] = "QNX";
    img->exifData()["Exif.Image.Model"] = "SDP 8.0";
    img->exifData()["Exif.Photo.ISOSpeedRatings"] = uint16_t(400);
    img->iptcData()["Iptc.Application2.Caption"] = "port test";
    img->xmpData()["Xmp.dc.title"] = "qnx-oss-ports";
    img->writeMetadata();

    Exiv2::DataBuf buf = img->io().read(static_cast<size_t>(img->io().size()));
    QT_CHECK(buf.size() > sizeof kJpeg, "metadata made the file bigger");

    auto back = Exiv2::ImageFactory::open(buf.c_data(), buf.size());
    back->readMetadata();
    const Exiv2::ExifData &ed = back->exifData();
    auto make = ed.findKey(Exiv2::ExifKey("Exif.Image.Make"));
    QT_CHECK(make != ed.end() && make->toString() == "QNX", "Exif.Image.Make survives");
    auto iso = ed.findKey(Exiv2::ExifKey("Exif.Photo.ISOSpeedRatings"));
    QT_CHECK(iso != ed.end() && iso->toInt64() == 400, "a numeric EXIF tag survives");
    const Exiv2::IptcData &id = back->iptcData();
    QT_CHECK(id.findKey(Exiv2::IptcKey("Iptc.Application2.Caption")) != id.end(),
             "IPTC caption survives");
    const Exiv2::XmpData &xd = back->xmpData();
    QT_CHECK(xd.findKey(Exiv2::XmpKey("Xmp.dc.title")) != xd.end(),
             "XMP title survives (expat is wired up)");
  } catch (const Exiv2::Error &e) {
    printf("  info exiv2 error: %s\n", e.what());
    QT_CHECK(false, "no exception while round-tripping metadata");
  }
  Exiv2::XmpParser::terminate();
  QT_END();
}
