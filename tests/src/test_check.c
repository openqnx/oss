/* check (unit-test framework): run a suite with known outcomes -- passing
 * assertions, a deliberate failure, an expected signal, an expected exit
 * code, an unexpected abort (an error), a checked fixture -- in fork mode,
 * and again in no-fork mode, and verify the counts and messages. Then
 * checkmk (an awk script, run directly through its #! line) turns a .check
 * file into C. */
#include "common.h"
#include <check.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static int fixture_value = 0;
static void setup(void) { fixture_value = 1234; }
static void teardown(void) { fixture_value = 0; }

START_TEST(t_pass) {
  ck_assert_int_eq(2 + 2, 4);
  ck_assert_str_eq("qnx", "qnx");
  ck_assert_uint_gt(10u, 3u);
  ck_assert_ptr_null(NULL);
}
END_TEST
START_TEST(t_fail) { ck_assert_msg(0, "deliberate failure %d", 42); }
END_TEST
START_TEST(t_signal) { raise(SIGUSR1); }
END_TEST
START_TEST(t_exit) { exit(3); }
END_TEST
START_TEST(t_abort) { abort(); }
END_TEST
START_TEST(t_fixture) { ck_assert_int_eq(fixture_value, 1234); }
END_TEST
START_TEST(t_loop) { ck_assert_int_lt(_i, 3); }
END_TEST

static Suite *make_suite(int forking) {
  Suite *s = suite_create("qnx");
  TCase *tc = tcase_create("core");
  tcase_add_test(tc, t_pass);
  tcase_add_test(tc, t_fail);
  tcase_add_loop_test(tc, t_loop, 0, 3);
  if (forking) {
    tcase_add_test_raise_signal(tc, t_signal, SIGUSR1);
    tcase_add_exit_test(tc, t_exit, 3);
    tcase_add_test(tc, t_abort);
  }
  suite_add_tcase(s, tc);
  TCase *tf = tcase_create("fixture");
  tcase_add_checked_fixture(tf, setup, teardown);
  tcase_add_test(tf, t_fixture);
  suite_add_tcase(s, tf);
  return s;
}

static int count(TestResult **r, int n, int type) {
  int c = 0;
  for (int i = 0; i < n; i++) if (tr_rtype(r[i]) == type) c++;
  return c;
}

int main(void) {
  QT_BEGIN("check");
  printf("  info check %d.%d.%d\n", check_major_version, check_minor_version, check_micro_version);
  QT_CHECK(check_major_version == 0 && check_minor_version == 15 && check_micro_version == 2, "library version 0.15.2");

  /* fork mode: pass, fail, 3 loop iterations, signal, exit, abort, fixture */
  SRunner *sr = srunner_create(make_suite(1));
  srunner_set_fork_status(sr, CK_FORK);
  srunner_run_all(sr, CK_SILENT);
  int run = srunner_ntests_run(sr), failed = srunner_ntests_failed(sr);
  printf("  info fork mode: %d run, %d failed\n", run, failed);
  QT_CHECK(run == 9, "fork mode: 9 tests run");
  QT_CHECK(failed == 2, "fork mode: 2 not passed (deliberate failure + abort)");
  TestResult **r = srunner_results(sr);
  QT_CHECK(count(r, run, CK_PASS) == 7 && count(r, run, CK_FAILURE) == 1 && count(r, run, CK_ERROR) == 1,
           "fork mode: 7 pass, 1 failure, 1 error");
  int msg_ok = 0, sig_ok = 0;
  for (int i = 0; i < run; i++) {
    const char *m = tr_msg(r[i]);
    if (tr_rtype(r[i]) == CK_FAILURE && m && strcmp(m, "deliberate failure 42") == 0) msg_ok = 1;
    if (tr_rtype(r[i]) == CK_ERROR && m) { printf("  info error result: %s\n", m); if (strstr(m, "signal")) sig_ok = 1; }
  }
  QT_CHECK(msg_ok, "failure message is \"deliberate failure 42\"");
  QT_CHECK(sig_ok, "abort() reported as an error by signal");
  free(r);
  srunner_free(sr);

  /* no-fork mode: pass, fail, 3 loop iterations, fixture */
  sr = srunner_create(make_suite(0));
  srunner_set_fork_status(sr, CK_NOFORK);
  srunner_run_all(sr, CK_SILENT);
  run = srunner_ntests_run(sr); failed = srunner_ntests_failed(sr);
  printf("  info no-fork mode: %d run, %d failed\n", run, failed);
  QT_CHECK(run == 6 && failed == 1, "no-fork mode: 6 run, 1 failed");
  srunner_free(sr);

  /* checkmk: an awk program, run directly as a user would. Its #! line must
     name an awk that exists on the target (it once named the build machine's). */
  char sb[256] = "";
  FILE *ck = fopen("/usr/local/bin/checkmk", "r");
  if (ck) { if (!fgets(sb, sizeof sb, ck)) sb[0] = 0; fclose(ck); }
  sb[strcspn(sb, "\n")] = 0;
  printf("  info checkmk interpreter line: %s\n", sb);
  char ip[256] = "";
  if (sscanf(sb, "#! %255s", ip) != 1) sscanf(sb, "#!%255s", ip);
  QT_CHECK(ip[0] == '/' && access(ip, X_OK) == 0, "checkmk's #! interpreter exists on the target");
  char dir[] = "/tmp/qnxport-check.XXXXXX";
  if (!mkdtemp(dir)) { QT_CHECK(0, "mkdtemp"); QT_END(); }
  char in[256], out[256], cmd[1024], line[512];
  snprintf(in, sizeof in, "%s/arith.check", dir);
  snprintf(out, sizeof out, "%s/arith.c", dir);
  FILE *f = fopen(in, "w");
  if (f) { fputs("#suite Arith\n\n#tcase Core\n\n#test add_works\n    ck_assert_int_eq(1 + 1, 2);\n\n"
                 "#test sub_works\n    ck_assert_int_eq(5 - 3, 2);\n", f); fclose(f); }
  snprintf(cmd, sizeof cmd, "/usr/local/bin/checkmk '%s' > '%s' 2>&1", in, out);
  int rc = system(cmd);
  int have_suite = 0, have_t1 = 0, have_t2 = 0, have_add = 0, have_run = 0;
  f = fopen(out, "r");
  while (f && fgets(line, sizeof line, f)) {
    if (strstr(line, "suite_create(\"Arith\")")) have_suite = 1;
    if (strstr(line, "START_TEST(add_works)")) have_t1 = 1;
    if (strstr(line, "START_TEST(sub_works)")) have_t2 = 1;
    if (strstr(line, "tcase_add_test(tc1_1, sub_works);")) have_add = 1;
    if (strstr(line, "srunner_run_all(sr, CK_ENV);")) have_run = 1;
  }
  if (f) fclose(f);
  QT_CHECK(rc == 0 && have_suite && have_t1 && have_t2 && have_add && have_run,
           "checkmk generated suite, two START_TESTs, tcase_add_test and srunner_run_all");
  unlink(in); unlink(out); rmdir(dir);
  QT_END();
}
