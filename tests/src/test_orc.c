/* Orc: compile a small SIMD program at run time and execute it. This is
 * just-in-time code generation, so it is entirely architecture-specific. */
#include "common.h"
#include <stdlib.h>
#include <orc/orc.h>
int main(void) {
  QT_BEGIN("orc");
  orc_init();
  printf("  info target = %s\n",
         orc_target_get_name(orc_target_get_default()) ?
         orc_target_get_name(orc_target_get_default()) : "(none)");
  OrcProgram *p = orc_program_new();
  QT_CHECK(p != NULL, "orc_program_new");
  orc_program_set_name(p, "add_s16");
  orc_program_add_destination(p, 2, "d1");
  orc_program_add_source(p, 2, "s1");
  orc_program_add_source(p, 2, "s2");
  orc_program_append_str(p, "addw", "d1", "s1", "s2");
  OrcCompileResult res = orc_program_compile(p);
  QT_CHECK(ORC_COMPILE_RESULT_IS_SUCCESSFUL(res), "orc_program_compile");
  printf("  info compile result = %d\n", res);
  short a[64], b[64], d[64];
  for (int i = 0; i < 64; ++i) { a[i] = (short)i; b[i] = (short)(2 * i); }
  OrcExecutor ex;
  orc_executor_set_program(&ex, p);
  orc_executor_set_n(&ex, 64);
  orc_executor_set_array_str(&ex, "d1", d);
  orc_executor_set_array_str(&ex, "s1", a);
  orc_executor_set_array_str(&ex, "s2", b);
  orc_executor_run(&ex);
  int ok = 1;
  for (int i = 0; i < 64; ++i) if (d[i] != (short)(3 * i)) { ok = 0; break; }
  QT_CHECK(ok, "the generated program computed s1 + s2 for every element");
  orc_program_free(p);
  QT_END();
}
