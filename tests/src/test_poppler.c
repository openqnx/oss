#include "common.h"
#include <string.h>
#include <unistd.h>
#include <cairo.h>
#include <cairo-pdf.h>
#include <poppler.h>
/* poppler through its GLib API: a PDF written here with cairo -- one A6
 * page with known text -- opened and read back. Known answers: one page,
 * its size in points, the text, and where it is on the page. */
int main(void) {
  QT_BEGIN("poppler");
  char path[] = "/tmp/poppler-XXXXXX.pdf"; int fd = mkstemps(path, 4); close(fd);
  cairo_surface_t *s = cairo_pdf_surface_create(path, 298, 420);
  cairo_t *cr = cairo_create(s);
  cairo_select_font_face(cr, "DejaVu Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_set_font_size(cr, 20); cairo_move_to(cr, 40, 100); cairo_show_text(cr, "Hello QNX poppler");
  cairo_show_page(cr); cairo_destroy(cr); cairo_surface_destroy(s);
  printf("  info poppler %s\n", poppler_get_version());
  char uri[300]; snprintf(uri, sizeof uri, "file://%s", path);
  GError *e = NULL;
  PopplerDocument *d = poppler_document_new_from_file(uri, NULL, &e);
  QT_CHECK(d != NULL, "poppler_document_new_from_file");
  if (!d) { printf("  info %s\n", e ? e->message : "?"); unlink(path); QT_END(); }
  QT_CHECK(poppler_document_get_n_pages(d) == 1, "one page");
  PopplerPage *p = poppler_document_get_page(d, 0);
  double w, h; poppler_page_get_size(p, &w, &h);
  printf("  info page %.0f x %.0f pt\n", w, h);
  QT_CHECK((int)(w + .5) == 298 && (int)(h + .5) == 420, "page size 298 x 420 pt (A6)");
  char *txt = poppler_page_get_text(p);
  printf("  info text: %s\n", txt ? txt : "(none)");
  QT_CHECK(txt && strstr(txt, "Hello QNX poppler"), "text read back");

  GList *found = poppler_page_find_text(p, "QNX");
  QT_CHECK(found != NULL, "find_text QNX");
  if (found) {
    PopplerRectangle *r = found->data;   /* PDF coordinates: origin bottom left */
    printf("  info QNX at x %.0f..%.0f, y %.0f..%.0f\n", r->x1, r->x2, r->y1, r->y2);
    QT_CHECK(r->x1 > 40 && r->x2 < 298 && r->y1 > 420 - 110 && r->y2 < 420 - 70, "found where it was drawn");
  }

  g_list_free_full(found, (GDestroyNotify)poppler_rectangle_free);
  g_free(txt); g_object_unref(p); g_object_unref(d); unlink(path);
  QT_END();
}
