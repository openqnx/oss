/* Enchant: the spell-checking broker, and the aspell provider it loads.
 *
 * The point of interest on QNX is the provider: enchant_aspell.so is dlopened
 * at broker construction from $libdir/enchant-2.  If the module is missing,
 * built without exported symbols, or fails to resolve, the broker still
 * constructs and simply reports no providers -- so enumerating them is the
 * check that actually proves the plugin mechanism works here.
 *
 * No dictionary is installed on the target, so this deliberately does not ask
 * for one: enchant_broker_dict_exists() answering at all is what we verify. */
#include "common.h"
#include <string.h>
#include <enchant.h>

struct seen { int aspell; int total; };

static void describe(const char *name, const char *desc,
                     const char *file, void *ud) {
  struct seen *s = (struct seen *)ud;
  (void)desc; (void)file;
  ++s->total;
  printf("  info provider %s (%s)\n", name ? name : "?", file ? file : "-");
  if (name && strcmp(name, "aspell") == 0) s->aspell = 1;
}

int main(void) {
  QT_BEGIN("enchant");

  printf("  info enchant %s\n", enchant_get_version());

  EnchantBroker *b = enchant_broker_init();
  QT_CHECK(b != NULL, "enchant_broker_init");
  if (!b) QT_END();

  struct seen s = { 0, 0 };
  enchant_broker_describe(b, describe, &s);
  QT_CHECK(s.total > 0, "broker enumerated at least one provider");
  QT_CHECK(s.aspell == 1, "the aspell provider module loaded");

  /* Asking about a dictionary must not crash and must not leave an error set
     on the broker; 0 (absent) is the expected answer with no dictionaries. */
  int have = enchant_broker_dict_exists(b, "en_US");
  printf("  info en_US dictionary present: %s\n", have ? "yes" : "no");
  QT_CHECK(have == 0 || have == 1, "enchant_broker_dict_exists returned a boolean");

  if (have) {
    EnchantDict *d = enchant_broker_request_dict(b, "en_US");
    QT_CHECK(d != NULL, "enchant_broker_request_dict(en_US)");
    if (d) {
      /* enchant_dict_check returns 0 for a correctly spelled word. */
      QT_CHECK(enchant_dict_check(d, "hello", 5) == 0, "\"hello\" spells correctly");
      QT_CHECK(enchant_dict_check(d, "helllo", 6) != 0, "\"helllo\" does not");
      enchant_broker_free_dict(b, d);
    }
  }

  enchant_broker_free(b);
  QT_END();
}
