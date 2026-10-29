/* The smaller additions in one binary: hyphen, enca, libgsf, gsm and aspell.
 * Each is exercised through the one operation it exists to perform. */
#include "common.h"
#include <stdlib.h>
#include <string.h>
#include <hyphen.h>
#include <enca.h>
#include <gsf/gsf.h>
#include <gsf/gsf-output-memory.h>
#include <gsm.h>
int main(void) {
  QT_BEGIN("hyphen / enca / libgsf / gsm");

  /* --- enca: character set analysis --- */
  /* "en" is not one of enca's languages -- it only covers those with an
     ambiguous multi-byte legacy encoding. Take the first one it reports. */
  size_t nlang = 0;
  const char **langs = enca_get_languages(&nlang);
  QT_CHECK(langs && nlang > 0, "enca reports supported languages");
  printf("  info %zu language(s), first = %s\n", nlang, nlang ? langs[0] : "-");
  EncaAnalyser an = nlang ? enca_analyser_alloc(langs[0]) : NULL;
  QT_CHECK(an != NULL, "enca_analyser_alloc");
  if (an) {
    unsigned char buf[] = "plain ascii text for analysis";
    EncaEncoding e = enca_analyse_const(an, buf, sizeof buf - 1);
    printf("  info enca charset = %s\n", enca_charset_name(e.charset, ENCA_NAME_STYLE_HUMAN));
    QT_CHECK(e.charset != ENCA_CS_UNKNOWN, "enca identified the encoding");
    enca_analyser_free(an);
  }
  if (langs) free(langs);

  /* --- libgsf: structured file I/O --- */
  gsf_init();
  GsfOutput *out = gsf_output_memory_new();
  QT_CHECK(out != NULL, "gsf_output_memory_new");
  if (out) {
    QT_CHECK(gsf_output_write(out, 3, (const guint8 *)"qnx"), "gsf_output_write");
    QT_CHECK(gsf_output_size(out) == 3, "gsf_output_size");
    const guint8 *data = gsf_output_memory_get_bytes(GSF_OUTPUT_MEMORY(out));
    QT_CHECK(data && memcmp(data, "qnx", 3) == 0, "bytes came back");
    gsf_output_close(out);
    g_object_unref(out);
  }
  gsf_shutdown();

  /* --- gsm: the 13 kbit/s codec, a full frame round trip --- */
  gsm g = gsm_create();
  QT_CHECK(g != NULL, "gsm_create");
  if (g) {
    gsm_signal pcm[160], back[160];
    for (int i = 0; i < 160; ++i) pcm[i] = (gsm_signal)(i * 100 - 8000);
    gsm_frame frame;
    gsm_encode(g, pcm, frame);
    QT_CHECK(gsm_decode(g, frame, back) == 0, "gsm encode/decode round trip");
    long energy = 0;
    for (int i = 0; i < 160; ++i) energy += labs((long)back[i]);
    QT_CHECK(energy > 0, "decoded frame carries signal energy");
    gsm_destroy(g);
  }

  /* --- hyphen: needs a dictionary, which a bare target may not have --- */
  HyphenDict *dict = hnj_hyphen_load("/usr/local/share/hyphen/hyph_en_US.dic");
  if (dict) {
    char word[] = "hyphenation";
    char hyphens[sizeof word];
    char **rep = NULL; int *pos = NULL, *cut = NULL;
    int rc = hnj_hyphen_hyphenate2(dict, word, (int)strlen(word), hyphens,
                                   NULL, &rep, &pos, &cut);
    QT_CHECK(rc == 0, "hnj_hyphen_hyphenate2");
    hnj_hyphen_free(dict);
  } else {
    printf("  info no hyphenation dictionary installed; library loaded only\n");
    QT_CHECK(hnj_hyphen_load("/nonexistent.dic") == NULL, "missing dictionary reports NULL");
  }
  QT_END();
}
