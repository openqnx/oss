/* libgpg-error: error codes and their texts (known strings), errno mapping,
 * the gpgrt estream API on a memory stream (printf, seek, read back), base64
 * encoding, string helpers, and the gpg-error(1) program decoding a code. */
#include "common.h"
#include <gpg-error.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

static int run(const char *cmd, char *out, size_t n) {
  FILE *p = popen(cmd, "r");
  size_t used = 0; out[0] = 0;
  if (!p) return -1;
  char b[512];
  while (fgets(b, sizeof b, p)) { size_t l = strlen(b); if (used + l < n) { memcpy(out + used, b, l + 1); used += l; } }
  return pclose(p);
}

int main(void) {
  QT_BEGIN("libgpg-error");
  gpgrt_init();
  const char *v = gpgrt_check_version(NULL);
  printf("  info gpgrt_check_version: %s\n", v ? v : "(null)");
  QT_CHECK(v && strcmp(v, "1.50") == 0, "library version 1.50");
  QT_CHECK(gpgrt_check_version("1.40") != NULL, "check_version(\"1.40\") satisfied");
  QT_CHECK(gpgrt_check_version("9.0") == NULL, "check_version(\"9.0\") not satisfied");

  QT_CHECK(strcmp(gpg_strerror(GPG_ERR_GENERAL), "General error") == 0, "strerror(GPG_ERR_GENERAL) == \"General error\"");
  QT_CHECK(strcmp(gpg_strerror(GPG_ERR_BAD_SIGNATURE), "Bad signature") == 0, "strerror(GPG_ERR_BAD_SIGNATURE)");
  QT_CHECK(strcmp(gpg_strerror(0), "Success") == 0, "strerror(0) == \"Success\"");
  gpg_error_t e = gpg_err_make(GPG_ERR_SOURCE_GCRYPT, GPG_ERR_BAD_SIGNATURE);
  QT_CHECK(gpg_err_code(e) == GPG_ERR_BAD_SIGNATURE && gpg_err_source(e) == GPG_ERR_SOURCE_GCRYPT,
           "gpg_err_make: code and source round trip");
  QT_CHECK(strcmp(gpg_strsource(e), "gcrypt") == 0, "strsource(GCRYPT) == \"gcrypt\"");
  char buf[128];
  QT_CHECK(gpg_strerror_r(GPG_ERR_NO_DATA, buf, sizeof buf) == 0 && strcmp(buf, "No data") == 0, "strerror_r(GPG_ERR_NO_DATA)");
  QT_CHECK(gpg_err_code_from_errno(ENOENT) == GPG_ERR_ENOENT, "errno ENOENT -> GPG_ERR_ENOENT");
  QT_CHECK(gpg_err_code_to_errno(GPG_ERR_ENOENT) == ENOENT, "GPG_ERR_ENOENT -> errno ENOENT");
  QT_CHECK(gpg_err_code_from_errno(EINVAL) == GPG_ERR_EINVAL, "errno EINVAL -> GPG_ERR_EINVAL");
  printf("  info strerror(GPG_ERR_ENOENT): %s\n", gpg_strerror(GPG_ERR_ENOENT));

  /* estream on memory */
  gpgrt_stream_t s = gpgrt_fopenmem(0, "w+");
  QT_CHECK(s != NULL, "gpgrt_fopenmem");
  if (s) {
    /* estream's own printf pads floats with spaces only (pr_float ignores
       the 0 flag, upstream and on every platform): use a plain width. */
    gpgrt_fprintf(s, "%s=%d;%5.1f;%-4x|\n", "answer", 42, 3.14159, 48879);
    gpgrt_rewind(s);
    char line[128] = "";
    gpgrt_fgets(line, sizeof line, s);
    printf("  info read back: %s", line);
    QT_CHECK(strcmp(line, "answer=42;  3.1;beef|\n") == 0, "fprintf to memory stream, rewind, fgets");
    gpgrt_fclose(s);
  }
  char *str = NULL;
  QT_CHECK(gpgrt_asprintf(&str, "%d-%s", 7, "up") == 4 && str && strcmp(str, "7-up") == 0, "gpgrt_asprintf");
  gpgrt_free(str);
  str = gpgrt_strconcat("qnx", "-", "8.0", NULL);
  QT_CHECK(str && strcmp(str, "qnx-8.0") == 0, "gpgrt_strconcat");
  gpgrt_free(str);
  str = gpgrt_fnameconcat("/tmp", "a", "b.txt", NULL);
  QT_CHECK(str && strcmp(str, "/tmp/a/b.txt") == 0, "gpgrt_fnameconcat");
  gpgrt_free(str);

  /* base64 into a memory stream */
  s = gpgrt_fopenmem(0, "w+");
  gpgrt_b64state_t b64 = s ? gpgrt_b64enc_start(s, NULL) : NULL;
  QT_CHECK(b64 != NULL, "gpgrt_b64enc_start");
  if (b64) {
    QT_CHECK(gpgrt_b64enc_write(b64, "hello QNX", 9) == 0 && gpgrt_b64enc_finish(b64) == 0, "b64enc write/finish");
    gpgrt_rewind(s);
    char enc[256] = ""; size_t n = gpgrt_fread(enc, 1, sizeof enc - 1, s); enc[n] = 0;
    printf("  info base64: %s", enc);
    QT_CHECK(strstr(enc, "aGVsbG8gUU5Y") != NULL, "base64(\"hello QNX\") == aGVsbG8gUU5Y");
  }
  if (s) gpgrt_fclose(s);

  /* gpg-error(1) */
  const char *prog = access("/usr/local/bin/gpg-error", X_OK) == 0 ? "/usr/local/bin/gpg-error" : "gpg-error";
  char out[1024], cmd[512];
  snprintf(cmd, sizeof cmd, "%s 8 2>&1", prog);
  int rc = run(cmd, out, sizeof out);
  printf("  info gpg-error 8: %s", out);
  QT_CHECK(rc == 0 && strstr(out, "Bad signature"), "gpg-error 8 -> Bad signature");
  snprintf(cmd, sizeof cmd, "%s %u 2>&1", prog, (unsigned)e);
  rc = run(cmd, out, sizeof out);
  printf("  info gpg-error %u: %s", (unsigned)e, out);
  QT_CHECK(rc == 0 && strstr(out, "gcrypt") && strstr(out, "Bad signature"), "gpg-error decodes source and code of a combined value");
  snprintf(cmd, sizeof cmd, "%s GPG_ERR_NO_DATA 2>&1", prog);
  rc = run(cmd, out, sizeof out);
  printf("  info gpg-error GPG_ERR_NO_DATA: %s", out);
  QT_CHECK(rc == 0 && strstr(out, "No data"), "gpg-error by symbol name");
  QT_END();
}
