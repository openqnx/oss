/* libdazzle: its non-widget building blocks -- the fuzzy search index
 * (matching, scoring, removal, highlight markup), the prefix trie, the
 * priority heap, the fixed-size ring buffer, keyword pattern specs and the
 * time-span formatter -- all usable without a display. */
#include "common.h"
#include <dazzle.h>

static gboolean count_cb(DzlTrie *t, const gchar *k, gpointer v, gpointer ud) {
  (void)t; (void)k; (void)v; ++*(int *)ud; return FALSE;
}
static int cmpint(gconstpointer a, gconstpointer b) { return *(const gint *)a - *(const gint *)b; }
static void sum_cb(gpointer data, gpointer ud) { *(int *)ud += *(int *)data; }

int main(void) {
  QT_BEGIN("libdazzle");
  printf("  info libdazzle %d.%d.%d\n", DZL_MAJOR_VERSION, DZL_MINOR_VERSION, DZL_MICRO_VERSION);

  DzlFuzzyMutableIndex *fz = dzl_fuzzy_mutable_index_new(FALSE);
  const char *keys[] = {"gtk_widget_show", "gtk_widget_hide", "g_object_unref", "gtk_window_new"};
  dzl_fuzzy_mutable_index_begin_bulk_insert(fz);
  for (int i = 0; i < 4; ++i) dzl_fuzzy_mutable_index_insert(fz, keys[i], GINT_TO_POINTER(i + 1));
  dzl_fuzzy_mutable_index_end_bulk_insert(fz);
  GArray *m = dzl_fuzzy_mutable_index_match(fz, "gws", 10);
  QT_CHECK(m && m->len == 1 && strcmp(g_array_index(m, DzlFuzzyMutableIndexMatch, 0).key, "gtk_widget_show") == 0 &&
           GPOINTER_TO_INT(g_array_index(m, DzlFuzzyMutableIndexMatch, 0).value) == 1, "\"gws\" matches only gtk_widget_show");
  if (m) g_array_unref(m);
  m = dzl_fuzzy_mutable_index_match(fz, "GTKW", 10);
  QT_CHECK(m && m->len == 3, "case-insensitive \"GTKW\" matches the three gtk_ keys");
  if (m && m->len >= 2)
    QT_CHECK(g_array_index(m, DzlFuzzyMutableIndexMatch, 0).score >= g_array_index(m, DzlFuzzyMutableIndexMatch, 1).score,
             "matches sorted by score");
  if (m) g_array_unref(m);
  dzl_fuzzy_mutable_index_remove(fz, "gtk_widget_show");
  m = dzl_fuzzy_mutable_index_match(fz, "gws", 10);
  QT_CHECK(m && m->len == 0 && !dzl_fuzzy_mutable_index_contains(fz, "gtk_widget_show"), "removed key no longer matches");
  if (m) g_array_unref(m);
  dzl_fuzzy_mutable_index_unref(fz);
  gchar *hl = dzl_fuzzy_highlight("gtk_widget_show", "gws", FALSE);
  printf("  info highlight %s\n", hl);
  QT_CHECK(hl && strcmp(hl, "<b>g</b>tk_<b>w</b>idget_<b>s</b>how") == 0, "highlight markup");
  g_free(hl);

  DzlTrie *t = dzl_trie_new(NULL);
  dzl_trie_insert(t, "car", GINT_TO_POINTER(1));
  dzl_trie_insert(t, "cart", GINT_TO_POINTER(2));
  dzl_trie_insert(t, "carbon", GINT_TO_POINTER(3));
  dzl_trie_insert(t, "dog", GINT_TO_POINTER(4));
  QT_CHECK(GPOINTER_TO_INT(dzl_trie_lookup(t, "cart")) == 2 && dzl_trie_lookup(t, "ca") == NULL, "trie lookup");
  int cnt = 0;
  dzl_trie_traverse(t, "car", G_PRE_ORDER, G_TRAVERSE_LEAVES, -1, count_cb, &cnt);
  QT_CHECK(cnt == 3, "three keys under the prefix \"car\"");
  QT_CHECK(dzl_trie_remove(t, "cart") && dzl_trie_lookup(t, "cart") == NULL && GPOINTER_TO_INT(dzl_trie_lookup(t, "car")) == 1,
           "remove keeps the prefix key");
  dzl_trie_unref(t);

  DzlHeap *h = dzl_heap_new(sizeof(gint), cmpint);
  gint vals[] = {5, 1, 9, 3, 7};
  dzl_heap_insert_vals(h, vals, 5);
  QT_CHECK(h->len == 5 && dzl_heap_peek(h, gint) == 9, "heap top is the maximum");
  gint prev = 100, v, ok = 1, n = 0;
  while (dzl_heap_extract(h, &v)) { if (v > prev) ok = 0; prev = v; ++n; }
  QT_CHECK(ok && n == 5, "extraction is non-increasing");
  dzl_heap_unref(h);

  DzlRing *r = dzl_ring_sized_new(sizeof(gint), 3, NULL);
  for (gint i = 1; i <= 5; ++i) dzl_ring_append_val(r, i);
  int sum = 0;
  dzl_ring_foreach(r, sum_cb, &sum);
  QT_CHECK(sum == 3 + 4 + 5, "ring keeps the last three of five values");
  dzl_ring_unref(r);

  DzlPatternSpec *ps = dzl_pattern_spec_new("foo bar");
  QT_CHECK(dzl_pattern_spec_match(ps, "the foo and the bar") && !dzl_pattern_spec_match(ps, "only foo here"),
           "pattern spec needs every keyword");
  dzl_pattern_spec_unref(ps);

  gchar *a = dzl_g_time_span_to_label(3 * G_TIME_SPAN_MINUTE + 30 * G_TIME_SPAN_SECOND);
  gchar *b = dzl_g_time_span_to_label(2 * G_TIME_SPAN_DAY + 3 * G_TIME_SPAN_HOUR + 6 * G_TIME_SPAN_MINUTE + 20 * G_TIME_SPAN_SECOND);
  QT_CHECK(strcmp(a, "03:30") == 0 && strcmp(b, "51:06:20") == 0, "time-span labels");
  g_free(a); g_free(b);
  QT_END();
}
