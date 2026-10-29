// TagLib: write and read an ID3v2 tag on an in-memory MPEG stream.
#include "common.h"
#include <taglib/tbytevectorstream.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/taglib.h>
int main() {
  QT_BEGIN("taglib");
  QT_CHECK(TAGLIB_MAJOR_VERSION == 2, "TagLib 2.x");
  TagLib::ByteVectorStream s{TagLib::ByteVector()};
  TagLib::MPEG::File f(&s, false);
  f.ID3v2Tag(true)->setTitle("QNX"); f.ID3v2Tag()->setYear(2026);
  QT_CHECK(f.save(), "save tag");
  TagLib::MPEG::File g(&s, false);
  QT_CHECK(g.ID3v2Tag() && g.ID3v2Tag()->title() == "QNX" && g.ID3v2Tag()->year() == 2026, "read tag back");
  QT_END();
}
