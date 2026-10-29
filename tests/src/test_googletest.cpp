/* googletest validates itself: this binary IS a gtest suite. */
#include <gtest/gtest.h>
#include <vector>
#include <string>
TEST(QnxPort, ArithmeticAndContainers) {
  EXPECT_EQ(2 + 2, 4);
  std::vector<int> v{3, 1, 2};
  std::sort(v.begin(), v.end());
  EXPECT_EQ(v.front(), 1);
  EXPECT_EQ(v.back(), 3);
}
TEST(QnxPort, StringsAndExceptions) {
  std::string s = "qnx";
  EXPECT_EQ(s + "-8.0", "qnx-8.0");
  EXPECT_THROW(({ std::vector<int> e; (void)e.at(5); }), std::out_of_range);
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  int rc = RUN_ALL_TESTS();
  printf(rc == 0 ? "PASS googletest\n" : "FAIL googletest\n");
  return rc;
}
