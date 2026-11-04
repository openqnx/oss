/* mate-common: <what this checks>. Link it the way an application would and check
 * behaviour, not just that a symbol exists: a call that does real work and
 * a result compared against a known answer. */
#include "common.h"
/* #include <mate-common.h> */

int main(void) {
  QT_BEGIN("mate-common");
  QT_CHECK(1, "replace with a real check");
  QT_END();
}
