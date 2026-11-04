/* spirv-headers: the C header's constants are the SPIR-V specification's
 * (magic number 0x07230203, OpCapability 17, OpTypeFloat 22, the Shader
 * capability 1), and the installed machine-readable core grammar -- the file
 * SPIRV-Tools and glslang are generated from -- is on the target and names
 * the same magic number and opcodes. */
#include "common.h"
#include <spirv/unified1/spirv.h>
#include <stdlib.h>

int main(void) {
  QT_BEGIN("spirv-headers");
  QT_CHECK(SpvMagicNumber == 0x07230203, "SpvMagicNumber is 0x07230203");
  QT_CHECK(SpvOpCapability == 17 && SpvOpTypeFloat == 22 && SpvOpFunction == 54, "opcodes Capability 17, TypeFloat 22, Function 54");
  QT_CHECK(SpvCapabilityShader == 1 && SpvExecutionModelFragment == 4, "Shader capability 1, Fragment execution model 4");
  QT_CHECK(SpvVersion >= 0x10600, "header version is SPIR-V 1.6 or later");
  const char *paths[] = { "/usr/local/include/spirv/unified1/spirv.core.grammar.json",
                          "/data/oss/usr/local/include/spirv/unified1/spirv.core.grammar.json" };
  FILE *f = NULL;
  for (int i = 0; i < 2 && !f; i++) f = fopen(paths[i], "r");
  QT_CHECK(f != NULL, "spirv.core.grammar.json is installed");
  if (f) {
    static char buf[4 << 20];
    size_t n = fread(buf, 1, sizeof buf - 1, f); buf[n] = 0; fclose(f);
    QT_CHECK(n > 100000, "the grammar is complete (over 100 kB)");
    QT_CHECK(strstr(buf, "\"magic_number\" : \"0x07230203\"") != NULL, "the grammar's magic number matches the header");
    QT_CHECK(strstr(buf, "\"opname\" : \"OpTypeFloat\"") && strstr(buf, "\"opcode\" : 22"), "the grammar lists OpTypeFloat (opcode 22)");
  }
  QT_END();
}
