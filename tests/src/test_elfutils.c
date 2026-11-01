/* elfutils libelf: reads the test's own executable (ELF class, machine,
 * the section names through the section-header string table, a symbol
 * from .symtab/.dynsym), then writes a new ELF file from scratch with
 * elf_newehdr/elf_newscn/elf_update, reads it back and checks every
 * field; elf_getdata_rawchunk and elf_end cover the raw-chunk search tree. */
#include <libelf.h>
#include <gelf.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "common.h"

#if defined(__x86_64__)
#define MY_MACHINE EM_X86_64
#elif defined(__aarch64__)
#define MY_MACHINE EM_AARCH64
#else
#define MY_MACHINE EM_NONE
#endif

static const char strtab[] = "\0.shstrtab\0.qnxdata\0";   /* offsets 1 and 11 */
static const char payload[] = "hello from libelf on QNX";

int main(int argc, char **argv)
{
  int fd;
  Elf *e;
  GElf_Ehdr eh;
  size_t shstrndx = 0, nsec = 0;
  Elf_Scn *scn = NULL;
  int has_text = 0, has_sym = 0, symfound = 0;
  char path[] = "/tmp/qnxport-libelf.XXXXXX";
  (void)argc;
  QT_BEGIN("elfutils (libelf)");
  QT_CHECK(elf_version(EV_CURRENT) != EV_NONE, "elf_version(EV_CURRENT)");

  /* ---- read our own executable */
  fd = open(argv[0], O_RDONLY);
  if (fd < 0) fd = open("/proc/self/exefile", O_RDONLY);
  QT_CHECK(fd >= 0, "open own executable");
  e = elf_begin(fd, ELF_C_READ, NULL);
  QT_CHECK(e != NULL && elf_kind(e) == ELF_K_ELF, "elf_begin: ELF_K_ELF");
  QT_CHECK(gelf_getehdr(e, &eh) != NULL && eh.e_ident[EI_CLASS] == ELFCLASS64 && eh.e_machine == MY_MACHINE,
           "ELF64 for this machine");
  QT_CHECK(eh.e_ident[EI_DATA] == ELFDATA2LSB && (eh.e_type == ET_EXEC || eh.e_type == ET_DYN), "little-endian executable");
  QT_CHECK(elf_getshdrstrndx(e, &shstrndx) == 0 && elf_getshdrnum(e, &nsec) == 0 && nsec > 5, "section count and string table");
  while ((scn = elf_nextscn(e, scn)) != NULL) {
    GElf_Shdr sh;
    const char *name;
    if (gelf_getshdr(scn, &sh) == NULL) continue;
    name = elf_strptr(e, shstrndx, sh.sh_name);
    if (name && strcmp(name, ".text") == 0 && sh.sh_type == SHT_PROGBITS && (sh.sh_flags & SHF_EXECINSTR)) has_text = 1;
    if (sh.sh_type == SHT_SYMTAB || sh.sh_type == SHT_DYNSYM) {
      Elf_Data *d = elf_getdata(scn, NULL);
      size_t i, n = sh.sh_entsize ? sh.sh_size / sh.sh_entsize : 0;
      has_sym = 1;
      for (i = 0; d && i < n; i++) {
        GElf_Sym s;
        const char *sn;
        if (gelf_getsym(d, (int)i, &s) == NULL) continue;
        sn = elf_strptr(e, sh.sh_link, s.st_name);
        if (sn && (strcmp(sn, "elf_begin") == 0 || strcmp(sn, "main") == 0)) symfound = 1;
      }
    }
  }
  QT_CHECK(has_text, ".text found by name, executable PROGBITS");
  QT_CHECK(has_sym && symfound, "symbol table names elf_begin or main");
  {
    Elf_Data *raw = elf_getdata_rawchunk(e, 0, 4, ELF_T_BYTE);
    QT_CHECK(raw && raw->d_size == 4 && memcmp(raw->d_buf, ELFMAG, 4) == 0, "elf_getdata_rawchunk: the ELF magic");
  }
  QT_CHECK(elf_end(e) == 0, "elf_end (frees the raw-chunk tree)");
  close(fd);

  /* ---- write a new ELF file */
  fd = mkstemp(path);
  QT_CHECK(fd >= 0, "temp file");
  e = elf_begin(fd, ELF_C_WRITE, NULL);
  {
    Elf64_Ehdr *h = elf64_newehdr(e);
    Elf_Scn *s1, *s2;
    Elf_Data *d;
    Elf64_Shdr *sh;
    QT_CHECK(h != NULL, "elf64_newehdr");
    h->e_ident[EI_DATA] = ELFDATA2LSB;
    h->e_type = ET_REL;
    h->e_machine = MY_MACHINE;
    s1 = elf_newscn(e);
    d = elf_newdata(s1);
    d->d_buf = (void *)payload; d->d_size = sizeof payload; d->d_type = ELF_T_BYTE; d->d_align = 1;
    sh = elf64_getshdr(s1);
    sh->sh_name = 11; sh->sh_type = SHT_PROGBITS; sh->sh_flags = SHF_ALLOC;
    s2 = elf_newscn(e);
    d = elf_newdata(s2);
    d->d_buf = (void *)strtab; d->d_size = sizeof strtab; d->d_type = ELF_T_BYTE; d->d_align = 1;
    sh = elf64_getshdr(s2);
    sh->sh_name = 1; sh->sh_type = SHT_STRTAB;
    h->e_shstrndx = (Elf64_Half)elf_ndxscn(s2);
    QT_CHECK(elf_update(e, ELF_C_WRITE) > 0, "elf_update writes the file");
    elf_end(e);
    close(fd);
  }

  /* ---- and read it back */
  fd = open(path, O_RDONLY);
  e = elf_begin(fd, ELF_C_READ, NULL);
  QT_CHECK(e && gelf_getehdr(e, &eh) && eh.e_type == ET_REL && eh.e_machine == MY_MACHINE, "read back: ET_REL, machine");
  {
    int found = 0;
    scn = NULL;
    elf_getshdrstrndx(e, &shstrndx);
    while ((scn = elf_nextscn(e, scn)) != NULL) {
      GElf_Shdr sh;
      const char *name;
      Elf_Data *d;
      gelf_getshdr(scn, &sh);
      name = elf_strptr(e, shstrndx, sh.sh_name);
      d = elf_getdata(scn, NULL);
      if (name && strcmp(name, ".qnxdata") == 0 && d && d->d_size == sizeof payload && memcmp(d->d_buf, payload, sizeof payload) == 0)
        found = 1;
    }
    QT_CHECK(found, "read back: section .qnxdata with the payload");
  }
  elf_end(e);
  close(fd);
  unlink(path);
  printf("  info elf_errmsg(-1): %s\n", elf_errmsg(-1) ? elf_errmsg(-1) : "(none)");
  QT_END();
}
