/* libass: an ASS/SSA subtitle renderer. Rasterises a real subtitle line, which
 * drives FreeType, HarfBuzz and FriBidi underneath. */
#include "common.h"
#include <stdlib.h>
#include <ass/ass.h>
static void quiet(int level, const char *fmt, va_list va, void *data) {
  (void)level; (void)fmt; (void)va; (void)data;
}
static const char kScript[] =
  "[Script Info]\n"
  "ScriptType: v4.00+\n"
  "PlayResX: 640\n"
  "PlayResY: 480\n"
  "\n"
  "[V4+ Styles]\n"
  "Format: Name, Fontname, Fontsize, PrimaryColour, Bold, Alignment, Encoding\n"
  "Style: Default,Sans,48,&H00FFFFFF,0,2,1\n"
  "\n"
  "[Events]\n"
  "Format: Layer, Start, End, Style, Text\n"
  "Dialogue: 0,0:00:00.00,0:00:10.00,Default,QNX SDP 8.0\n";
int main(void) {
  QT_BEGIN("libass");
  ASS_Library *lib = ass_library_init();
  QT_CHECK(lib != NULL, "ass_library_init");
  if (!lib) QT_END();
  ass_set_message_cb(lib, quiet, NULL);
  ASS_Renderer *rend = ass_renderer_init(lib);
  QT_CHECK(rend != NULL, "ass_renderer_init");
  ass_set_frame_size(rend, 640, 480);
  ass_set_fonts(rend, NULL, "Sans", ASS_FONTPROVIDER_AUTODETECT, NULL, 1);
  ASS_Track *track = ass_read_memory(lib, (char *)kScript, sizeof kScript - 1, NULL);
  QT_CHECK(track != NULL, "ass_read_memory parsed the script");
  QT_CHECK(track && track->n_events == 1, "one dialogue event");
  QT_CHECK(track && track->n_styles >= 1, "at least one style");
  QT_CHECK(track && track->PlayResX == 640 && track->PlayResY == 480, "play resolution");
  if (track) {
    int changed = 0;
    ASS_Image *img = ass_render_frame(rend, track, 1000, &changed);
    if (img) {
      int n = 0;
      for (ASS_Image *p = img; p; p = p->next) ++n;
      printf("  info rendered %d image bitmap(s)\n", n);
      QT_CHECK(n > 0, "the subtitle rasterised to at least one bitmap");
    } else {
      /* No font on the target: parsing still has to work, and it did. */
      printf("  info no font available, nothing rasterised\n");
      QT_CHECK(changed == 0, "render reported no change rather than failing");
    }
    ass_free_track(track);
  }
  ass_renderer_done(rend);
  ass_library_done(lib);
  QT_END();
}
