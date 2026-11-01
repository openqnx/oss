/* OpenPAM: a PAM transaction against policies written for the test, with
 * the modules this port installs (pam_permit, pam_deny, pam_return) named
 * by path. The application's conversation function supplies the user name
 * (pam_get_user prompts through it) and a password (pam_get_authtok); the
 * policy decides the result of pam_authenticate / pam_acct_mgmt. */
#include <security/pam_appl.h>
#include <security/openpam.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "common.h"

static int conv_calls;
static char last_prompt[128];

static int conv(int n, const struct pam_message **msg, struct pam_response **resp, void *data)
{
  struct pam_response *r = calloc((size_t)n, sizeof *r);
  int i;
  (void)data;
  if (r == NULL) return PAM_BUF_ERR;
  for (i = 0; i < n; i++) {
    conv_calls++;
    snprintf(last_prompt, sizeof last_prompt, "%s", msg[i]->msg ? msg[i]->msg : "");
    if (msg[i]->msg_style == PAM_PROMPT_ECHO_ON) r[i].resp = strdup("alice");
    else if (msg[i]->msg_style == PAM_PROMPT_ECHO_OFF) r[i].resp = strdup("s3cret");
  }
  *resp = r;
  return PAM_SUCCESS;
}

static const char *moddir(void)
{
  static const char *dirs[] = { "/usr/local/lib", "/data/oss/usr/local/lib", NULL };
  int i;
  char p[256];
  if (getenv("QT_PAM_MODDIR")) return getenv("QT_PAM_MODDIR");
  for (i = 0; dirs[i]; i++) {
    snprintf(p, sizeof p, "%s/pam_permit.so.2", dirs[i]);
    if (access(p, R_OK) == 0) return dirs[i];
  }
  return "/usr/local/lib";
}

/* Write a pam.d-style policy file; returns its path. */
static const char *policy(const char *dir, const char *name, const char *auth, const char *acct)
{
  static char path[512];
  FILE *f;
  snprintf(path, sizeof path, "%s/%s", dir, name);
  f = fopen(path, "w");
  if (f == NULL) return NULL;
  /* All four facilities: a policy that leaves one out gets it from the
     system's "other" policy, which is not under the test's control. */
  fprintf(f, "auth\trequired\t%s/%s\naccount\trequired\t%s/%s\n"
             "session\trequired\t%s/pam_permit.so.2\npassword\trequired\t%s/pam_permit.so.2\n",
          moddir(), auth, moddir(), acct, moddir(), moddir());
  fclose(f);
  return path;
}

int main(void)
{
  struct pam_conv pc = { conv, NULL };
  pam_handle_t *ph = NULL;
  const char *user = NULL, *tok = NULL, *svc;
  const void *item = NULL;
  char dir[] = "/tmp/qnxport-openpam.XXXXXX";
  int rc;
  QT_BEGIN("openpam");
  printf("  info OpenPAM %d (%s), modules from %s\n", OPENPAM_VERSION, OPENPAM_RELEASE, moddir());
  QT_CHECK(mkdtemp(dir) != NULL, "temp dir");
  chmod(dir, 0755);
  /* The policies live in a temp dir (world-writable /tmp is above it):
     turn off the ownership/permission checks of policy and module files. */
  openpam_set_feature(OPENPAM_VERIFY_POLICY_FILE, 0);
  openpam_set_feature(OPENPAM_VERIFY_MODULE_FILE, 0);
  openpam_set_feature(OPENPAM_RESTRICT_SERVICE_NAME, 0);
  openpam_set_feature(OPENPAM_RESTRICT_MODULE_NAME, 0);

  svc = policy(dir, "permit", "pam_permit.so.2", "pam_permit.so.2");
  rc = pam_start(svc, NULL, &pc, &ph);
  QT_CHECK(rc == PAM_SUCCESS && ph != NULL, "pam_start with a policy file path");
  if (rc != PAM_SUCCESS || ph == NULL) { printf("    pam_start: %d %s\n", rc, pam_strerror(NULL, rc)); QT_END(); }
  rc = pam_get_user(ph, &user, "login: ");
  QT_CHECK(rc == PAM_SUCCESS && user && strcmp(user, "alice") == 0 && strcmp(last_prompt, "login: ") == 0,
           "pam_get_user asked the conversation (\"login: \") and got alice");
  rc = pam_get_authtok(ph, PAM_AUTHTOK, &tok, "Password: ");
  QT_CHECK(rc == PAM_SUCCESS && tok && strcmp(tok, "s3cret") == 0, "pam_get_authtok: password through the conversation");
  QT_CHECK(conv_calls == 2, "two conversation calls");
  QT_CHECK(pam_get_item(ph, PAM_USER, &item) == PAM_SUCCESS && item && strcmp(item, "alice") == 0, "PAM_USER item");
  QT_CHECK(pam_set_item(ph, PAM_RHOST, "127.0.0.1") == PAM_SUCCESS && pam_get_item(ph, PAM_RHOST, &item) == PAM_SUCCESS
           && strcmp(item, "127.0.0.1") == 0, "set/get PAM_RHOST");
  QT_CHECK(pam_putenv(ph, "QNXPAM=yes") == PAM_SUCCESS && pam_getenv(ph, "QNXPAM") && strcmp(pam_getenv(ph, "QNXPAM"), "yes") == 0,
           "pam_putenv / pam_getenv");
  rc = pam_authenticate(ph, 0);
  QT_CHECK(rc == PAM_SUCCESS, "pam_authenticate: pam_permit -> PAM_SUCCESS");
  QT_CHECK(pam_acct_mgmt(ph, 0) == PAM_SUCCESS, "pam_acct_mgmt: pam_permit -> PAM_SUCCESS");
  QT_CHECK(pam_end(ph, rc) == PAM_SUCCESS, "pam_end");

  svc = policy(dir, "deny", "pam_deny.so.2", "pam_permit.so.2");
  ph = NULL;
  rc = pam_start(svc, "bob", &pc, &ph);
  rc = rc == PAM_SUCCESS ? pam_authenticate(ph, 0) : rc;
  QT_CHECK(rc == PAM_AUTH_ERR, "pam_authenticate: pam_deny -> PAM_AUTH_ERR");
  if (ph) pam_end(ph, rc);

  svc = policy(dir, "return", "pam_return.so.2 error=PAM_USER_UNKNOWN", "pam_return.so.2 error=PAM_ACCT_EXPIRED");
  ph = NULL;
  rc = pam_start(svc, "carol", &pc, &ph);
  if (rc != PAM_SUCCESS) { printf("    pam_start: %d\n", rc); QT_END(); }
  QT_CHECK(pam_authenticate(ph, 0) == PAM_USER_UNKNOWN, "pam_return error=PAM_USER_UNKNOWN (module option)");
  QT_CHECK(pam_acct_mgmt(ph, 0) == PAM_ACCT_EXPIRED, "pam_return error=PAM_ACCT_EXPIRED");
  pam_end(ph, PAM_SUCCESS);

  ph = NULL;
  rc = pam_start("/nonexistent/policy", "dave", &pc, &ph);
  QT_CHECK(rc != PAM_SUCCESS, "pam_start refuses a policy file that does not exist");
  if (rc == PAM_SUCCESS && ph) pam_end(ph, rc);
  QT_CHECK(strcmp(pam_strerror(NULL, PAM_AUTH_ERR), "Authentication error") == 0, "pam_strerror(PAM_AUTH_ERR) = \"Authentication error\"");

  unlink(policy(dir, "permit", "x", "x")); unlink(policy(dir, "deny", "x", "x")); unlink(policy(dir, "return", "x", "x"));
  rmdir(dir);
  QT_END();
}
