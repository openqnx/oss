/* iso-codes: the ISO 3166 / 639 / 4217 / 15924 tables and their
 * translations, as GTK, GNOME and installers read them. Parse the JSON
 * files (json-c) and look up known entries -- countries by alpha-2 and
 * numeric code, a subdivision, a currency, a language, a script -- then read
 * the gettext catalogues (.mo) for German and French and translate country
 * and language names, the way dgettext("iso_3166-1", ...) does. */
#include "common.h"
#include <json-c/json.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

static char base[512];

static json_object *load(const char *file, const char *key, int *n) {
  char p[700]; snprintf(p, sizeof p, "%s/iso-codes/json/%s", base, file);
  json_object *root = json_object_from_file(p), *arr = NULL;
  if (root && json_object_object_get_ex(root, key, &arr) && json_object_is_type(arr, json_type_array)) { *n = (int)json_object_array_length(arr); return arr; }
  *n = 0; return NULL;
}
static const char *str(json_object *o, const char *k) { json_object *v; return json_object_object_get_ex(o, k, &v) ? json_object_get_string(v) : NULL; }
static json_object *find(json_object *arr, const char *k, const char *v) {
  for (size_t i = 0; arr && i < json_object_array_length(arr); i++) {
    json_object *o = json_object_array_get_idx(arr, i); const char *s = str(o, k);
    if (s && !strcmp(s, v)) return o;
  }
  return NULL;
}
static int field_is(json_object *o, const char *k, const char *v) { const char *s = o ? str(o, k) : NULL; return s && !strcmp(s, v); }

/* a GNU .mo catalogue: look a message up the way gettext does (linear scan) */
static char *mo_lookup(const char *lang, const char *domain, const char *msgid, char *out, size_t outlen) {
  char p[700]; snprintf(p, sizeof p, "%s/locale/%s/LC_MESSAGES/%s.mo", base, lang, domain);
  FILE *f = fopen(p, "rb"); if (!f) return NULL;
  fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
  unsigned char *m = malloc(len); size_t got = fread(m, 1, len, f); fclose(f);
  char *res = NULL;
  if ((long)got == len && len > 28) {
    uint32_t magic = m[0] | m[1] << 8 | m[2] << 16 | (uint32_t)m[3] << 24; int le = magic == 0x950412deu;
#define U32(o) (le ? (uint32_t)(m[o] | m[(o)+1] << 8 | m[(o)+2] << 16 | (uint32_t)m[(o)+3] << 24) : (uint32_t)((uint32_t)m[o] << 24 | m[(o)+1] << 16 | m[(o)+2] << 8 | m[(o)+3]))
    if (le || magic == 0xde120495u) {
      uint32_t n = U32(8), oo = U32(12), to = U32(16);
      for (uint32_t i = 0; i < n && !res; i++) {
        uint32_t ol = U32(oo + 8 * i), op = U32(oo + 8 * i + 4);
        if (op + ol <= (uint32_t)len && ol == strlen(msgid) && !memcmp(m + op, msgid, ol)) {
          uint32_t tl = U32(to + 8 * i), tp = U32(to + 8 * i + 4);
          if (tp + tl <= (uint32_t)len) { snprintf(out, outlen, "%.*s", (int)tl, m + tp); res = out; }
        }
      }
    }
  }
  free(m);
  return res;
}

int main(void) {
  QT_BEGIN("iso-codes");
  const char *roots[] = { getenv("QNX_PORT_DATA_ROOT"), "/usr/local/share", "/data/oss/usr/local/share" };
  for (int i = 0; i < 3 && !base[0]; i++) if (roots[i]) {
    char p[600]; snprintf(p, sizeof p, "%s/iso-codes/json/iso_3166-1.json", roots[i]);
    if (access(p, R_OK) == 0) snprintf(base, sizeof base, "%s", roots[i]);
  }
  QT_CHECK(base[0], "share/iso-codes/json installed");
  if (!base[0]) QT_END();
  int n;
  json_object *c = load("iso_3166-1.json", "3166-1", &n);
  printf("  info ISO 3166-1: %d countries\n", n);
  QT_CHECK(c && n >= 249, "ISO 3166-1: 249 or more countries");
  json_object *de = find(c, "alpha_2", "DE");
  QT_CHECK(field_is(de, "name", "Germany") && field_is(de, "alpha_3", "DEU") && field_is(de, "numeric", "276"), "DE: Germany, DEU, 276");
  QT_CHECK(field_is(find(c, "numeric", "392"), "alpha_2", "JP") && field_is(find(c, "alpha_3", "CAN"), "name", "Canada"), "numeric 392 -> JP; CAN -> Canada");
  QT_CHECK(field_is(find(c, "alpha_2", "FR"), "official_name", "French Republic"), "FR official name: French Republic");
  json_object *s = load("iso_3166-2.json", "3166-2", &n);
  QT_CHECK(s && n > 4000 && field_is(find(s, "code", "DE-BY"), "name", "Bayern") && field_is(find(s, "code", "US-CA"), "name", "California"),
           "ISO 3166-2: DE-BY Bayern, US-CA California");
  json_object *cur = load("iso_4217.json", "4217", &n);
  QT_CHECK(cur && n > 150 && field_is(find(cur, "alpha_3", "EUR"), "numeric", "978") && field_is(find(cur, "alpha_3", "JPY"), "name", "Yen"),
           "ISO 4217: EUR 978, JPY Yen");
  json_object *l = load("iso_639-2.json", "639-2", &n);
  json_object *fr = find(l, "alpha_3", "fra");
  QT_CHECK(l && n > 400 && field_is(fr, "alpha_2", "fr") && field_is(fr, "bibliographic", "fre") && field_is(fr, "name", "French"), "ISO 639-2: fra = fr, fre, French");
  json_object *l3 = load("iso_639-3.json", "639-3", &n);
  QT_CHECK(l3 && n > 7000 && field_is(find(l3, "alpha_3", "deu"), "name", "German"), "ISO 639-3: 7000+ languages, deu German");
  json_object *sc = load("iso_15924.json", "15924", &n);
  QT_CHECK(sc && field_is(find(sc, "alpha_4", "Latn"), "numeric", "215") && field_is(find(sc, "alpha_4", "Cyrl"), "name", "Cyrillic"), "ISO 15924: Latn 215, Cyrl Cyrillic");
  char buf[128];
  const char *t1 = mo_lookup("de", "iso_3166-1", "Germany", buf, sizeof buf);
  printf("  info de: Germany -> %s\n", t1 ? t1 : "-");
  QT_CHECK(t1 && !strcmp(t1, "Deutschland"), "de catalogue: Germany -> Deutschland");
  t1 = mo_lookup("fr", "iso_639-2", "German", buf, sizeof buf);
  QT_CHECK(t1 && !strcmp(t1, "Allemand"), "fr catalogue: German -> Allemand");
  t1 = mo_lookup("fr", "iso_4217", "Euro", buf, sizeof buf);
  QT_CHECK(t1 && !strcmp(t1, "Euro"), "fr catalogue: Euro -> Euro");
  QT_END();
}
