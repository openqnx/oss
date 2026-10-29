/* libdatrie: build a double-array trie, store values, walk it back. */
#include "common.h"
#include <stdlib.h>
#include <datrie/trie.h>
static Bool collect(const AlphaChar *key, TrieData data, void *user) {
  (void)key;
  *(int *)user += (int)data;
  return TRUE;
}
int main(void) {
  QT_BEGIN("libdatrie");
  AlphaMap *map = alpha_map_new();
  QT_CHECK(map != NULL, "alpha_map_new");
  QT_CHECK(alpha_map_add_range(map, 0x0061, 0x007A) == 0, "add the a-z range");
  Trie *trie = trie_new(map);
  QT_CHECK(trie != NULL, "trie_new");
  alpha_map_free(map);
  if (!trie) QT_END();
  static const AlphaChar k1[] = { 'q','n','x',0 };
  static const AlphaChar k2[] = { 'q','n','o','d','e',0 };
  QT_CHECK(trie_store(trie, k1, 8), "trie_store(qnx)");
  QT_CHECK(trie_store(trie, k2, 34), "trie_store(qnode)");
  TrieData v = 0;
  QT_CHECK(trie_retrieve(trie, k1, &v) && v == 8, "trie_retrieve(qnx)");
  QT_CHECK(trie_retrieve(trie, k2, &v) && v == 34, "trie_retrieve(qnode)");
  static const AlphaChar missing[] = { 'z','z',0 };
  QT_CHECK(!trie_retrieve(trie, missing, &v), "an absent key is not found");
  int sum = 0;
  QT_CHECK(trie_enumerate(trie, collect, &sum), "trie_enumerate");
  QT_CHECK(sum == 42, "enumeration visited both entries");
  QT_CHECK(trie_delete(trie, k1), "trie_delete");
  QT_CHECK(!trie_retrieve(trie, k1, &v), "the deleted key is gone");
  trie_free(trie);
  QT_END();
}
