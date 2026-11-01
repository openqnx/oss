/* APR-util: known answers for base64, MD5, SHA-1 and the Apache password
 * hashes ($apr1$, and SHA-512 crypt through libxcrypt), UUID format/parse,
 * URI parsing, HTTP date parsing, XML parsing (expat), character set
 * conversion (apr_xlate, GNU libiconv), the sdbm DBM in a temporary
 * directory, a thread-safe queue, and the two modules apr-util dlopen()s
 * from $PREFIX/lib/apr-util-1: apr_crypto with OpenSSL (PBKDF2 key, AES-256-CBC
 * against a ciphertext computed with openssl enc) and the SQLite 3 DBD
 * driver on an in-memory database. */
#include "common.h"
#include <apr_general.h>
#include <apr_strings.h>
#include <apr_base64.h>
#include <apr_md5.h>
#include <apr_sha1.h>
#include <apr_uuid.h>
#include <apr_uri.h>
#include <apr_date.h>
#include <apr_xml.h>
#include <apr_xlate.h>
#include <apr_dbm.h>
#include <apr_queue.h>
#include <apr_crypto.h>
#include <apr_dbd.h>
#include <apu_version.h>
#include <stdlib.h>
#include <unistd.h>

static void hex(const unsigned char *d, int n, char *out) {
  int i; for (i = 0; i < n; i++) sprintf(out + 2 * i, "%02x", d[i]);
}

int main(void) {
  apr_pool_t *p;
  char buf[512], h[129];
  QT_BEGIN("apr-util");
  apr_initialize(); apr_pool_create(&p, NULL);
  printf("  info APR-util %s\n", apu_version_string());

  /* base64, MD5, SHA-1 */
  apr_base64_encode(buf, "QNX", 3);
  QT_CHECK(strcmp(buf, "UU5Y") == 0, "apr_base64_encode(QNX) = UU5Y");
  { unsigned char d[APR_MD5_DIGESTSIZE]; apr_md5(d, "abc", 3); hex(d, 16, h);
    QT_CHECK(strcmp(h, "900150983cd24fb0d6963f7d28e17f72") == 0, "apr_md5(abc)"); }
  { apr_sha1_ctx_t c; unsigned char d[APR_SHA1_DIGESTSIZE];
    apr_sha1_init(&c); apr_sha1_update(&c, "abc", 3); apr_sha1_final(d, &c); hex(d, 20, h);
    QT_CHECK(strcmp(h, "a9993e364706816aba3e25717850c26c9cd0d89d") == 0, "apr_sha1(abc)"); }

  /* passwords: APR's own MD5 crypt, and the system crypt() for $6$ */
  {
    char enc[120];
    QT_CHECK(apr_md5_encode("secret", "$apr1$qnxsalt8$", enc, sizeof enc) == APR_SUCCESS &&
             apr_password_validate("secret", enc) == APR_SUCCESS && apr_password_validate("wrong", enc) != APR_SUCCESS,
             "apr_md5_encode $apr1$ hash validates the right password only");
    QT_CHECK(apr_password_validate("secret",
             "$6$qnxsalt8$vI/mxwW/sFTiqWflRJhaqeTKIgrxeqbSAwjTA7QKzEz3cXXZED5F3NEMMiW3n1Iij3jnBc.QJyd1YoodHwEgm1") == APR_SUCCESS,
             "apr_password_validate: SHA-512 crypt hash from openssl passwd -6 (system crypt)");
  }

  /* UUIDs */
  {
    apr_uuid_t u, v; char s[APR_UUID_FORMATTED_LENGTH + 1], s2[APR_UUID_FORMATTED_LENGTH + 1];
    apr_uuid_get(&u); apr_uuid_format(s, &u);
    QT_CHECK(strlen(s) == 36 && s[8] == '-' && s[23] == '-', "apr_uuid_get + format: 36 characters");
    QT_CHECK(apr_uuid_parse(&v, s) == APR_SUCCESS && memcmp(&u, &v, 16) == 0, "apr_uuid_parse round trip");
    apr_uuid_get(&v); apr_uuid_format(s2, &v);
    QT_CHECK(strcmp(s, s2) != 0, "two apr_uuid_get() differ");
  }

  /* URI, date */
  {
    apr_uri_t u;
    QT_CHECK(apr_uri_parse(p, "https://user@example.com:8443/a/b?x=1#frag", &u) == APR_SUCCESS &&
             !strcmp(u.scheme, "https") && !strcmp(u.hostname, "example.com") && u.port == 8443 &&
             !strcmp(u.path, "/a/b") && !strcmp(u.query, "x=1") && !strcmp(u.fragment, "frag") && !strcmp(u.user, "user"),
             "apr_uri_parse splits scheme/user/host/port/path/query/fragment");
    QT_CHECK(apr_date_parse_http("Sun, 06 Nov 1994 08:49:37 GMT") == apr_time_from_sec(784111777),
             "apr_date_parse_http(RFC 1123 date) = 784111777 s");
  }

  /* XML through expat */
  {
    apr_xml_parser *xp = apr_xml_parser_create(p); apr_xml_doc *doc = NULL;
    const char *x = "<?xml version=\"1.0\"?><cfg version=\"2\"><item name=\"a\">one</item><item name=\"b\">two</item></cfg>";
    apr_status_t rv = apr_xml_parser_feed(xp, x, strlen(x));
    if (rv == APR_SUCCESS) rv = apr_xml_parser_done(xp, &doc);
    QT_CHECK(rv == APR_SUCCESS && doc && !strcmp(doc->root->name, "cfg"), "apr_xml parses a document, root <cfg>");
    if (doc) {
      apr_xml_elem *e = doc->root->first_child; int n = 0;
      for (; e; e = e->next) n++;
      QT_CHECK(n == 2 && doc->root->first_child->first_cdata.first &&
               !strcmp(doc->root->first_child->first_cdata.first->text, "one") &&
               !strcmp(doc->root->attr->value, "2"), "two <item> children, first text 'one', attribute version=2");
    }
    xp = apr_xml_parser_create(p);
    rv = apr_xml_parser_feed(xp, "<a><b></a>", 10);
    if (rv == APR_SUCCESS) rv = apr_xml_parser_done(xp, &doc);
    QT_CHECK(rv != APR_SUCCESS, "mismatched tags are an error");
  }

  /* character sets */
  {
    apr_xlate_t *xl; apr_size_t in = 6, out = sizeof buf; const char *src = "caf\xc3\xa9!";
    apr_status_t rv = apr_xlate_open(&xl, "ISO-8859-1", "UTF-8", p);
    QT_CHECK(rv == APR_SUCCESS, "apr_xlate_open(ISO-8859-1 <- UTF-8)");
    if (rv == APR_SUCCESS) {
      rv = apr_xlate_conv_buffer(xl, src, &in, buf, &out);
      QT_CHECK(rv == APR_SUCCESS && in == 0 && sizeof buf - out == 5 && !memcmp(buf, "caf\xe9!", 5), "UTF-8 'caf\xc3\xa9!' -> 5 bytes Latin-1");
      apr_xlate_close(xl);
    }
  }

  /* sdbm */
  {
    apr_dbm_t *db; apr_datum_t k, v; char path[256];
    snprintf(path, sizeof path, "/tmp/qnx-aprutil-%d", (int)getpid());
    QT_CHECK(apr_dbm_open_ex(&db, "SDBM", path, APR_DBM_RWCREATE, APR_OS_DEFAULT, p) == APR_SUCCESS, "apr_dbm_open_ex(SDBM)");
    k.dptr = "key"; k.dsize = 3; v.dptr = "value"; v.dsize = 5;
    QT_CHECK(apr_dbm_store(db, k, v) == APR_SUCCESS, "apr_dbm_store");
    apr_dbm_close(db);
    apr_dbm_open_ex(&db, "SDBM", path, APR_DBM_READONLY, APR_OS_DEFAULT, p);
    memset(&v, 0, sizeof v);
    QT_CHECK(apr_dbm_fetch(db, k, &v) == APR_SUCCESS && v.dsize == 5 && !memcmp(v.dptr, "value", 5), "reopen + apr_dbm_fetch");
    apr_dbm_close(db);
    snprintf(buf, sizeof buf, "%s.dir", path); unlink(buf);
    snprintf(buf, sizeof buf, "%s.pag", path); unlink(buf);
  }

  /* queue */
  {
    apr_queue_t *q; void *out = NULL; int a = 1, b = 2;
    apr_queue_create(&q, 4, p);
    apr_queue_push(q, &a); apr_queue_push(q, &b);
    QT_CHECK(apr_queue_size(q) == 2 && apr_queue_pop(q, &out) == APR_SUCCESS && out == &a, "apr_queue is FIFO");
  }

  /* apr_crypto with the OpenSSL module */
  {
    const apr_crypto_driver_t *drv = NULL; const apu_err_t *err = NULL; apr_crypto_t *f = NULL;
    apr_crypto_key_t *key = NULL; apr_size_t ivlen = 0, bs = 0, len = 0, len2 = 0;
    const unsigned char ivb[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}; const unsigned char *iv = ivb;
    apr_crypto_block_t *blk = NULL; unsigned char *ct = NULL, *pt = NULL;
    const char *msg = "QNX apr_crypto check"; apr_status_t rv;
    apr_crypto_init(p);
    rv = apr_crypto_get_driver(&drv, "openssl", NULL, &err, p);
    QT_CHECK(rv == APR_SUCCESS && drv, "apr_crypto_get_driver(openssl): module loaded from lib/apr-util-1");
    if (rv != APR_SUCCESS && err) printf("  info %s: %s\n", err->reason ? err->reason : "", err->msg ? err->msg : "");
    if (drv && apr_crypto_make(&f, drv, NULL, p) == APR_SUCCESS &&
        apr_crypto_passphrase(&key, &ivlen, "qnx-secret", 10, (const unsigned char *)"saltsalt", 8,
                              APR_KEY_AES_256, APR_MODE_CBC, 1, 4096, f, p) == APR_SUCCESS) {
      QT_CHECK(ivlen == 16, "AES-256-CBC key from a passphrase (PBKDF2, 4096 rounds), 16-byte IV");
      if (apr_crypto_block_encrypt_init(&blk, &iv, key, &bs, p) == APR_SUCCESS &&
          apr_crypto_block_encrypt(&ct, &len, (const unsigned char *)msg, strlen(msg), blk) == APR_SUCCESS &&
          apr_crypto_block_encrypt_finish(ct + len, &len2, blk) == APR_SUCCESS) {
        hex(ct, (int)(len + len2), h);
        QT_CHECK(len + len2 == 32 && !strcmp(h, "400230b844c817ea74fee5aac1d7dc031910fa2aa37e84df41d2dbe287aa07d6"),
                 "ciphertext = openssl enc -aes-256-cbc with the same key and IV");
        len = len2 = 0;
        if (apr_crypto_block_decrypt_init(&blk, &bs, ivb, key, p) == APR_SUCCESS &&
            apr_crypto_block_decrypt(&pt, &len, ct, 32, blk) == APR_SUCCESS &&
            apr_crypto_block_decrypt_finish(pt + len, &len2, blk) == APR_SUCCESS)
          QT_CHECK(len + len2 == strlen(msg) && !memcmp(pt, msg, strlen(msg)), "decrypts back to the message");
        else QT_CHECK(0, "decrypt");
      } else QT_CHECK(0, "encrypt");
    } else QT_CHECK(0, "apr_crypto_make + apr_crypto_passphrase");
  }

  /* DBD with the SQLite 3 module */
  {
    const apr_dbd_driver_t *drv = NULL; apr_dbd_t *db = NULL; apr_dbd_results_t *res = NULL;
    apr_dbd_row_t *row = NULL; int nrows = 0; const char *err = NULL; apr_status_t rv;
    apr_dbd_init(p);
    rv = apr_dbd_get_driver(p, "sqlite3", &drv);
    QT_CHECK(rv == APR_SUCCESS, "apr_dbd_get_driver(sqlite3): module loaded from lib/apr-util-1");
    if (rv == APR_SUCCESS && apr_dbd_open_ex(drv, p, ":memory:", &db, &err) == APR_SUCCESS) {
      apr_dbd_query(drv, db, &nrows, "CREATE TABLE t (k INTEGER, v TEXT)");
      apr_dbd_query(drv, db, &nrows, "INSERT INTO t VALUES (1, 'one'), (2, 'two'), (3, 'three')");
      QT_CHECK(nrows == 3, "INSERT of 3 rows reports 3");
      if (apr_dbd_select(drv, p, db, &res, "SELECT v FROM t WHERE k >= 2 ORDER BY k", 0) == 0) {
        char got[64] = "";
        while (apr_dbd_get_row(drv, p, res, &row, -1) == 0) { strcat(got, apr_dbd_get_entry(drv, row, 0)); strcat(got, ","); }
        QT_CHECK(!strcmp(got, "two,three,"), "SELECT ... ORDER BY returns two,three");
      } else QT_CHECK(0, "apr_dbd_select");
      apr_dbd_close(drv, db);
    } else QT_CHECK(0, err ? err : "apr_dbd_open_ex(:memory:)");
  }
  apr_pool_destroy(p); apr_terminate();
  QT_END();
}
