/* util-macros: <what this checks>. Link it the way an application would and check
 * behaviour, not just that a symbol exists: a call that does real work and
 * a result compared against a known answer. */
#include "common.h"
/* #include <util-macros.h> */

int main(void) {
  QT_BEGIN("util-macros");
  QT_CHECK(1, "replace with a real check");
  QT_END();
}
