/* libexif: build an EXIF block, serialise it, parse it back. */
#include "common.h"
#include <stdlib.h>
#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
int main(void) {
  QT_BEGIN("libexif");
  ExifData *ed = exif_data_new();
  QT_CHECK(ed != NULL, "exif_data_new");
  exif_data_set_byte_order(ed, EXIF_BYTE_ORDER_INTEL);
  exif_data_fix(ed);
  ExifEntry *e = exif_entry_new();
  exif_content_add_entry(ed->ifd[EXIF_IFD_0], e);
  exif_entry_initialize(e, EXIF_TAG_MAKE);
  static const char make[] = "QNX";
  e->data = (unsigned char *)strdup(make);
  e->size = sizeof make; e->components = sizeof make;
  e->format = EXIF_FORMAT_ASCII; e->tag = EXIF_TAG_MAKE;
  unsigned char *buf = NULL; unsigned int len = 0;
  exif_data_save_data(ed, &buf, &len);
  QT_CHECK(buf != NULL && len > 0, "exif_data_save_data");
  exif_entry_unref(e);
  exif_data_unref(ed);

  ExifData *back = exif_data_new_from_data(buf, len);
  QT_CHECK(back != NULL, "exif_data_new_from_data");
  ExifEntry *got = back ? exif_content_get_entry(back->ifd[EXIF_IFD_0], EXIF_TAG_MAKE) : NULL;
  QT_CHECK(got != NULL, "Make tag found after the round trip");
  if (got) {
    char val[64];
    exif_entry_get_value(got, val, sizeof val);
    QT_CHECK(strcmp(val, "QNX") == 0, "Make tag value is \"QNX\"");
  }
  QT_CHECK(back && exif_data_get_byte_order(back) == EXIF_BYTE_ORDER_INTEL,
           "byte order survives");
  QT_CHECK(strcmp(exif_tag_get_name(EXIF_TAG_MAKE), "Make") == 0, "exif_tag_get_name");
  if (back) exif_data_unref(back);
  free(buf);
  QT_END();
}
