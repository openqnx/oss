/* libxcrypt: hash known passwords with fixed salts for every enabled
 * method and compare with reference hashes (Drepper's SHA-crypt vector,
 * OpenWall bcrypt vector; the others from libxcrypt on Linux), verify by
 * re-hashing with the stored hash as the setting, reject a wrong password,
 * and generate fresh salts with crypt_gensalt_rn / check them. */
#include "common.h"
#include <crypt.h>
static const char *V[][3] = {
  { "sha512crypt", "Hello world!", "$6$saltstring$svn8UoSVapNtMuq1ukKS4tPQd8iKwSMHWjl/O817G3uBnIFNjnQJuesI68u4OTLiBFdcbYEdFCoEOfaS35inz1" },
  { "sha256crypt", "qnx-password", "$5$rounds=5000$toolongsaltstrin$x4s4XuNJkXHW.4j6JAqd7PX11Ie2Pj8GdNKiytHci27" },
  { "bcrypt 2a", "U*U", "$2a$05$CCCCCCCCCCCCCCCCCCCCC.E5YPO9kmyuRGyh0XouQYb4YMJKvyOeW" },
  { "bcrypt 2b", "U*U", "$2b$05$CCCCCCCCCCCCCCCCCCCCC.E5YPO9kmyuRGyh0XouQYb4YMJKvyOeW" },
  { "yescrypt", "qnx-password", "$y$j9T$saltsaltsaltsalt$Lt9Nnvq9HD/XQD/VDWVxIaT0i9iCejSa4l2Jvz4mAL4" },
  { "scrypt", "qnx", "$7$C6..../....SodiumChloride$pWrzrzvsUuSL3YKTP/eATjcdvx1pSFR.Cp.sFwWHn8A" },
  { "md5crypt", "password", "$1$saltstri$qQY4WxjABChYG1ccLpfkz/" },
  { "descrypt", "test", "abgOeLfPimXQo" },
};
static struct crypt_data cd;
int main(void) {
  QT_BEGIN("libxcrypt");
  char what[96];
  for (size_t i = 0; i < sizeof V / sizeof V[0]; i++) {
    /* the setting is the hash itself: crypt ignores the trailing checksum */
    memset(&cd, 0, sizeof cd);
    const char *r = crypt_r(V[i][1], V[i][2], &cd);
    if (!r || strcmp(r, V[i][2])) printf("  info %s got %s\n", V[i][0], r ? r : "(null)");
    snprintf(what, sizeof what, "%s: reference hash reproduced", V[i][0]);
    QT_CHECK(r && strcmp(r, V[i][2]) == 0, what);
  }
  memset(&cd, 0, sizeof cd);
  const char *r = crypt_r("Hello world?", V[0][2], &cd);
  QT_CHECK(r && r[0] != '*' && strcmp(r, V[0][2]) != 0, "wrong password gives a different hash");
  memset(&cd, 0, sizeof cd);
  r = crypt_r("x", "$99$nosuchmethod", &cd);
  QT_CHECK(r == NULL || r[0] == '*', "unknown method rejected (failure token)");

  char salt[CRYPT_GENSALT_OUTPUT_SIZE];
  const char *pfx[] = { "$y$", "$2b$", "$6$" };
  for (int i = 0; i < 3; i++) {
    char *s = crypt_gensalt_rn(pfx[i], 0, NULL, 0, salt, sizeof salt);   /* NULL rbytes: OS randomness */
    printf("  info gensalt %s -> %s\n", pfx[i], s ? s : "(null)");
    snprintf(what, sizeof what, "crypt_gensalt_rn %s + hash + verify", pfx[i]);
    int ok = s && strncmp(s, pfx[i], strlen(pfx[i])) == 0;
    if (ok) {
      char h[CRYPT_OUTPUT_SIZE];
      memset(&cd, 0, sizeof cd); r = crypt_r("secret", s, &cd); ok = r && r[0] != '*';
      if (ok) { strcpy(h, r); memset(&cd, 0, sizeof cd); r = crypt_r("secret", h, &cd); ok = r && strcmp(r, h) == 0; }
      if (ok) ok = crypt_checksalt(h) == CRYPT_SALT_OK;
    }
    QT_CHECK(ok, what);
  }
  const char *pref = crypt_preferred_method();
  printf("  info preferred method %s\n", pref ? pref : "(null)");
  QT_CHECK(pref && pref[0] == '$', "crypt_preferred_method");
  QT_END();
}
