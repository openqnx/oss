#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <whisper.h>
/* whisper.cpp: real inference on the CPU -- the Silero voice-activity model
 * (shipped with whisper.cpp) run over the JFK sample ("And so, my fellow
 * Americans ..."): speech must be found, starting in the first second and
 * ending by the eleventh, with the pause after "Americans" splitting it.
 * And the language table: 99 languages, en = 0, "fr" known. */
static float *load_wav(const char *p, int *n) {
  FILE *f = fopen(p, "rb"); if (!f) return NULL;
  fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);
  unsigned char *b = malloc(sz); if (fread(b, 1, sz, f) != (size_t)sz) { fclose(f); free(b); return NULL; } fclose(f);
  long off = 12; int16_t *pcm = NULL; long bytes = 0;
  while (off + 8 <= sz) {   /* find the "data" chunk */
    uint32_t len = b[off + 4] | b[off + 5] << 8 | b[off + 6] << 16 | (uint32_t)b[off + 7] << 24;
    if (!memcmp(b + off, "data", 4)) { pcm = (int16_t *)(b + off + 8); bytes = len; break; }
    off += 8 + len;
  }
  if (!pcm) { free(b); return NULL; }
  *n = (int)(bytes / 2); float *s = malloc(sizeof(float) * *n);
  for (int i = 0; i < *n; i++) s[i] = pcm[i] / 32768.0f;
  free(b); return s;
}
int main(void) {
  QT_BEGIN("whisper.cpp");
  printf("  info %s\n", whisper_print_system_info());
  QT_CHECK(whisper_lang_max_id() >= 98, "language table (99+ languages)");
  QT_CHECK(whisper_lang_id("en") == 0 && strcmp(whisper_lang_str(0), "en") == 0, "en is language 0");
  QT_CHECK(whisper_lang_id("fr") > 0, "fr known");
  const char *roots[] = { "/usr/local/opt/whisper.cpp/share", "/data/oss/usr/local/opt/whisper.cpp/share" };
  char model[256] = "", wav[256] = "";
  for (int i = 0; i < 2; i++) {
    snprintf(model, sizeof model, "%s/models/ggml-silero-v6.2.0.bin", roots[i]);
    snprintf(wav, sizeof wav, "%s/samples/jfk.wav", roots[i]);
    FILE *f = fopen(model, "rb"); if (f) { fclose(f); break; }
  }
  int n = 0; float *s = load_wav(wav, &n);
  QT_CHECK(s != NULL && n > 16000 * 10, "jfk.wav: 16 kHz, 11 s");
  struct whisper_vad_context_params cp = whisper_vad_default_context_params(); cp.n_threads = 2;
  struct whisper_vad_context *v = whisper_vad_init_from_file_with_params(model, cp);
  QT_CHECK(v != NULL, "Silero VAD model loaded");
  if (v && s) {
    struct whisper_vad_params vp = whisper_vad_default_params();
    struct whisper_vad_segments *sg = whisper_vad_segments_from_samples(v, vp, s, n);
    int k = sg ? whisper_vad_segments_n_segments(sg) : 0;
    for (int i = 0; i < k; i++)
      printf("  info speech %.2f s .. %.2f s\n", whisper_vad_segments_get_segment_t0(sg, i) / 100.0, whisper_vad_segments_get_segment_t1(sg, i) / 100.0);
    QT_CHECK(k >= 2, "speech found, in more than one segment");
    if (k >= 1) {
      QT_CHECK(whisper_vad_segments_get_segment_t0(sg, 0) < 100, "speech starts in the first second");
      float t1 = whisper_vad_segments_get_segment_t1(sg, k - 1);
      QT_CHECK(t1 > 900 && t1 <= 1110, "speech ends between 9 s and the end (11 s)");
    }
    if (sg) whisper_vad_free_segments(sg);
  }
  if (v) whisper_vad_free(v);
  free(s);
  QT_END();
}
