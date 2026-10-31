/* CUnit: build a registry with two suites (one with a fixture), run it with
 * the core runner, and check the run summary counts --
 * including a deliberately failing assertion, a failing suite initialiser
 * and the recorded failure's line and condition text. */
#include "common.h"
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <CUnit/TestRun.h>

static int fixture;
static int init_ok(void) { fixture = 41; return 0; }
static int clean_ok(void) { fixture = 0; return 0; }
static int init_bad(void) { return 1; }
static void t_pass(void) { CU_ASSERT_EQUAL(fixture + 1, 42); CU_ASSERT_STRING_EQUAL("qnx", "qnx"); CU_ASSERT_PTR_NOT_NULL(&fixture); }
static int fail_line;
static void t_fail(void) { CU_ASSERT(1 == 1); fail_line = __LINE__; CU_ASSERT(2 + 2 == 5); }
static void t_double(void) { CU_ASSERT_DOUBLE_EQUAL(3.14159, 3.1416, 0.001); }
static void t_never(void) { CU_ASSERT(0); }

int main(void) {
  QT_BEGIN("cunit");
  QT_CHECK(CU_initialize_registry() == CUE_SUCCESS, "registry initialised");
  CU_pSuite s1 = CU_add_suite("arith", init_ok, clean_ok);
  CU_pSuite s2 = CU_add_suite("broken-init", init_bad, NULL);
  QT_CHECK(s1 && s2, "two suites added");
  QT_CHECK(CU_add_test(s1, "passes", t_pass) && CU_add_test(s1, "fails", t_fail) && CU_add_test(s1, "double", t_double) &&
           CU_add_test(s2, "never runs", t_never), "four tests added");
  QT_CHECK(CU_add_test(s1, "no function", NULL) == NULL && CU_get_error() == CUE_NOTEST, "test without a function rejected");

  /* CU_run_all_tests: the plain runner, which prints nothing itself */
  CU_ErrorCode rc = CU_run_all_tests();
  printf("  info CU_run_all_tests: %d (%s)\n", (int)rc, CU_get_error_msg());
  unsigned run = CU_get_number_of_tests_run(), failed = CU_get_number_of_tests_failed();
  unsigned asserts = CU_get_number_of_asserts(), afail = CU_get_number_of_failures();
  unsigned suites_run = CU_get_number_of_suites_run(), suites_failed = CU_get_number_of_suites_failed();
  printf("  info tests run %u failed %u, asserts %u failures %u, suites run %u failed %u\n",
         run, failed, asserts, afail, suites_run, suites_failed);
  QT_CHECK(run == 3 && failed == 1, "3 tests run, 1 failed (suite with failing init skipped)");
  QT_CHECK(asserts == 6, "6 assertions evaluated");
  QT_CHECK(suites_run == 1 && suites_failed == 1, "1 suite run, 1 suite failed to initialise");
  QT_CHECK(fixture == 0, "suite cleanup ran");
  CU_pFailureRecord fr = CU_get_failure_list();
  int found = 0;
  for (; fr; fr = fr->pNext)
    if (fr->type == CUF_AssertFailed && fr->pTest && strcmp(fr->pTest->pName, "fails") == 0) {
      found = (int)fr->uiLineNumber == fail_line && strstr(fr->strCondition, "2 + 2 == 5") != NULL;
      printf("  info failure at line %u: %s\n", fr->uiLineNumber, fr->strCondition);
    }
  QT_CHECK(found, "failure record names the test, line and condition");
  QT_CHECK(CU_get_suite("arith") == s1 && CU_get_test(s1, "double") != NULL, "lookup by name");
  CU_cleanup_registry();
  QT_CHECK(CU_get_registry() == NULL, "registry cleaned up");
  QT_END();
}
