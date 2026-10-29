#include "common.h"
#include <gflags/gflags.h>
DEFINE_int32(ecu_id, 0, "ECU identifier");
DEFINE_string(mode, "idle", "operating mode");
int main(int argc, char **argv) {
  QT_BEGIN("gflags");
  /* Feed the parser a synthetic command line so the test is self-contained. */
  static char a0[] = "test_gflags", a1[] = "--ecu_id=7", a2[] = "--mode=drive";
  char *av[] = {a0, a1, a2, NULL}; int ac = 3; char **avp = av;
  gflags::ParseCommandLineFlags(&ac, &avp, true);
  QT_CHECK(FLAGS_ecu_id == 7, "int flag parsed");
  QT_CHECK(FLAGS_mode == "drive", "string flag parsed");
  (void)argc; (void)argv;
  QT_END();
}
