/* chrpath: the ELF program test_chrpath edits. Linked with a DT_RPATH of
 * /opt/qnx-chrpath-test/lib:/usr/local/lib (tests/run-level/
 * g7-buildtools.cmake) and needing libm, so that after chrpath changes or
 * deletes the path it must still load and run. Run alone, it checks itself. */
#include "common.h"
#include <math.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  volatile double two = argc > 1 ? atof(argv[1]) : 2.0;
  QT_BEGIN("chrpath_prog");
  QT_CHECK(fabs(sqrt(two) - 1.41421356237) < 1e-9, "loaded and ran: sqrt(2) from libm");
  QT_END();
}
