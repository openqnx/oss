#include "common.h"
#include <string.h>
#include <capstone/capstone.h>
/* capstone: disassemble known x86-64 and AArch64 instructions. */
int main(void) {
  QT_BEGIN("capstone");
  csh h; cs_insn *in;
  const uint8_t x86[] = { 0x55, 0x48, 0x89, 0xe5, 0x31, 0xc0, 0xc3 };
  QT_CHECK(cs_open(CS_ARCH_X86, CS_MODE_64, &h) == CS_ERR_OK, "cs_open x86-64");
  size_t n = cs_disasm(h, x86, sizeof x86, 0x1000, 0, &in);
  QT_CHECK(n == 4, "4 instructions");
  if (n == 4) {
    for (size_t i = 0; i < n; i++) printf("  info 0x%lx %s %s\n", (unsigned long)in[i].address, in[i].mnemonic, in[i].op_str);
    QT_CHECK(!strcmp(in[0].mnemonic, "push") && !strcmp(in[0].op_str, "rbp"), "push rbp");
    QT_CHECK(!strcmp(in[1].mnemonic, "mov") && !strcmp(in[1].op_str, "rbp, rsp"), "mov rbp, rsp");
    QT_CHECK(!strcmp(in[2].mnemonic, "xor") && !strcmp(in[2].op_str, "eax, eax"), "xor eax, eax");
    QT_CHECK(!strcmp(in[3].mnemonic, "ret") && in[3].address == 0x1006, "ret at 0x1006");
  }
  cs_free(in, n); cs_close(&h);
  const uint8_t a64[] = { 0xfd, 0x7b, 0xbf, 0xa9, 0x00, 0x00, 0x80, 0x52, 0xc0, 0x03, 0x5f, 0xd6 };
  QT_CHECK(cs_open(CS_ARCH_ARM64, CS_MODE_ARM, &h) == CS_ERR_OK, "cs_open AArch64");
  n = cs_disasm(h, a64, sizeof a64, 0, 0, &in);
  QT_CHECK(n == 3, "3 instructions");
  if (n == 3) {
    for (size_t i = 0; i < n; i++) printf("  info %s %s\n", in[i].mnemonic, in[i].op_str);
    QT_CHECK(!strcmp(in[0].mnemonic, "stp"), "stp x29, x30, [sp, #-0x10]!");
    QT_CHECK(!strcmp(in[1].mnemonic, "mov") && !strcmp(in[1].op_str, "w0, #0"), "mov w0, #0");
    QT_CHECK(!strcmp(in[2].mnemonic, "ret"), "ret");
  }
  cs_free(in, n); cs_close(&h);
  QT_END();
}
