/* aws-c-auth: SigV4 request signing against the AWS test suite's
 * "get-vanilla" case (known Authorization header), presigned query-string
 * signing, and the static and environment credentials providers. */
#include "common.h"
#include <stdlib.h>
#include <aws/common/common.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/common/date_time.h>
#include <aws/http/request_response.h>
#include <aws/auth/auth.h>
#include <aws/auth/credentials.h>
#include <aws/auth/signable.h>
#include <aws/auth/signing.h>
#include <aws/auth/signing_config.h>
#include <aws/auth/signing_result.h>

static struct aws_mutex mtx = AWS_MUTEX_INIT;
static struct aws_condition_variable cv = AWS_CONDITION_VARIABLE_INIT;
static int signed_done, sign_err, creds_done;
static struct aws_http_message *req;
static struct aws_allocator *al;
static char akid[64];
static bool p_signed(void *ud) { (void)ud; return signed_done; }
static bool p_creds(void *ud) { (void)ud; return creds_done; }
static void on_signed(struct aws_signing_result *r, int err, void *ud) {
  (void)ud;
  sign_err = err;
  if (!err) aws_apply_signing_result_to_http_request(req, al, r);
  aws_mutex_lock(&mtx); signed_done = 1; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx);
}
static void on_creds(struct aws_credentials *c, int err, void *ud) {
  (void)ud;
  akid[0] = 0;
  if (!err && c) {
    struct aws_byte_cursor k = aws_credentials_get_access_key_id(c);
    snprintf(akid, sizeof akid, "%.*s", (int)k.len, (const char *)k.ptr);
  }
  aws_mutex_lock(&mtx); creds_done = 1; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx);
}
static int header(const char *name, char *out, size_t n) {
  struct aws_byte_cursor v;
  if (aws_http_headers_get(aws_http_message_get_headers(req), aws_byte_cursor_from_c_str(name), &v)) return 0;
  snprintf(out, n, "%.*s", (int)v.len, (const char *)v.ptr);
  return 1;
}
static int sign(struct aws_signing_config_aws *cfg) {
  signed_done = 0; sign_err = -1;
  struct aws_signable *s = aws_signable_new_http_request(al, req);
  int rc = aws_sign_request_aws(al, s, (struct aws_signing_config_base *)cfg, on_signed, NULL);
  aws_mutex_lock(&mtx);
  aws_condition_variable_wait_for_pred(&cv, &mtx, 5LL * 1000000000LL, p_signed, NULL);
  aws_mutex_unlock(&mtx);
  aws_signable_destroy(s);
  return rc == AWS_OP_SUCCESS && signed_done && sign_err == 0;
}
static int get_creds(struct aws_credentials_provider *p) {
  creds_done = 0;
  aws_credentials_provider_get_credentials(p, on_creds, NULL);
  aws_mutex_lock(&mtx);
  aws_condition_variable_wait_for_pred(&cv, &mtx, 5LL * 1000000000LL, p_creds, NULL);
  aws_mutex_unlock(&mtx);
  return creds_done;
}

int main(void) {
  QT_BEGIN("aws-c-auth");
  al = aws_default_allocator();
  aws_auth_library_init(al);

  struct aws_credentials *cr = aws_credentials_new(al, aws_byte_cursor_from_c_str("AKIDEXAMPLE"),
      aws_byte_cursor_from_c_str("wJalrXUtnFEMI/K7MDENG+bPxRfiCYEXAMPLEKEY"), aws_byte_cursor_from_c_str(""), UINT64_MAX);
  QT_CHECK(cr != NULL, "static credentials");

  req = aws_http_message_new_request(al);
  aws_http_message_set_request_method(req, aws_byte_cursor_from_c_str("GET"));
  aws_http_message_set_request_path(req, aws_byte_cursor_from_c_str("/"));
  struct aws_http_header host = {aws_byte_cursor_from_c_str("Host"), aws_byte_cursor_from_c_str("example.amazonaws.com"), 0};
  aws_http_message_add_header(req, host);

  struct aws_signing_config_aws cfg;
  memset(&cfg, 0, sizeof cfg);
  cfg.config_type = AWS_SIGNING_CONFIG_AWS;
  cfg.algorithm = AWS_SIGNING_ALGORITHM_V4;
  cfg.signature_type = AWS_ST_HTTP_REQUEST_HEADERS;
  cfg.region = aws_byte_cursor_from_c_str("us-east-1");
  cfg.service = aws_byte_cursor_from_c_str("service");
  struct aws_byte_cursor dc = aws_byte_cursor_from_c_str("2015-08-30T12:36:00Z");
  aws_date_time_init_from_str_cursor(&cfg.date, &dc, AWS_DATE_FORMAT_ISO_8601);
  cfg.flags.use_double_uri_encode = true;
  cfg.flags.should_normalize_uri_path = true;
  cfg.signed_body_header = AWS_SBHT_NONE;
  cfg.credentials = cr;

  QT_CHECK(sign(&cfg), "SigV4 header signing completes");
  char auth[512] = "", date[64] = "";
  header("Authorization", auth, sizeof auth);
  header("X-Amz-Date", date, sizeof date);
  printf("  info Authorization: %s\n", auth);
  QT_CHECK(strcmp(date, "20150830T123600Z") == 0, "X-Amz-Date header added");
  QT_CHECK(strcmp(auth, "AWS4-HMAC-SHA256 Credential=AKIDEXAMPLE/20150830/us-east-1/service/aws4_request, "
                        "SignedHeaders=host;x-amz-date, "
                        "Signature=5fa00fa31553b73ebf1942676e86291e8372ff2a2260956d9b8aae1d763fbf31") == 0,
           "Authorization matches the AWS SigV4 test suite (get-vanilla)");
  aws_http_message_release(req);

  /* Presigned URL: the signature moves into the query string. */
  req = aws_http_message_new_request(al);
  aws_http_message_set_request_method(req, aws_byte_cursor_from_c_str("GET"));
  aws_http_message_set_request_path(req, aws_byte_cursor_from_c_str("/bucket/key.txt"));
  struct aws_http_header h2 = {aws_byte_cursor_from_c_str("Host"), aws_byte_cursor_from_c_str("s3.amazonaws.com"), 0};
  aws_http_message_add_header(req, h2);
  cfg.signature_type = AWS_ST_HTTP_REQUEST_QUERY_PARAMS;
  cfg.service = aws_byte_cursor_from_c_str("s3");
  cfg.flags.use_double_uri_encode = false;
  cfg.signed_body_value = g_aws_signed_body_value_unsigned_payload;
  cfg.expiration_in_seconds = 3600;
  QT_CHECK(sign(&cfg), "SigV4 query-string signing completes");
  struct aws_byte_cursor path;
  aws_http_message_get_request_path(req, &path);
  char p[1024]; snprintf(p, sizeof p, "%.*s", (int)path.len, (const char *)path.ptr);
  printf("  info presigned path: %.80s...\n", p);
  QT_CHECK(strstr(p, "X-Amz-Algorithm=AWS4-HMAC-SHA256") && strstr(p, "X-Amz-Expires=3600") &&
           strstr(p, "X-Amz-Credential=AKIDEXAMPLE%2F20150830%2Fus-east-1%2Fs3%2Faws4_request"),
           "presigned query has algorithm, expiry and credential scope");
  const char *sig = strstr(p, "X-Amz-Signature=");
  QT_CHECK(sig && strlen(sig + 16) >= 64, "presigned query carries a 64-hex-digit signature");
  aws_http_message_release(req);
  aws_credentials_release(cr);

  struct aws_credentials_provider_static_options so;
  memset(&so, 0, sizeof so);
  so.access_key_id = aws_byte_cursor_from_c_str("AKIASTATIC");
  so.secret_access_key = aws_byte_cursor_from_c_str("secret");
  struct aws_credentials_provider *sp = aws_credentials_provider_new_static(al, &so);
  QT_CHECK(sp && get_creds(sp) && strcmp(akid, "AKIASTATIC") == 0, "static credentials provider");
  aws_credentials_provider_release(sp);

  setenv("AWS_ACCESS_KEY_ID", "AKIAFROMENV", 1);
  setenv("AWS_SECRET_ACCESS_KEY", "envsecret", 1);
  struct aws_credentials_provider_environment_options eo;
  memset(&eo, 0, sizeof eo);
  struct aws_credentials_provider *ep = aws_credentials_provider_new_environment(al, &eo);
  QT_CHECK(ep && get_creds(ep) && strcmp(akid, "AKIAFROMENV") == 0, "environment credentials provider");
  aws_credentials_provider_release(ep);

  aws_auth_library_clean_up();
  QT_END();
}
