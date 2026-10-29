#include "common.h"
#include <glog/logging.h>
int main(int argc, char **argv) {
  QT_BEGIN("glog");
  google::InitGoogleLogging(argv[0]);
  FLAGS_logtostderr = true;
  LOG(INFO) << "glog reached stderr from QNX";
  LOG(WARNING) << "warning path";
  QT_CHECK(true, "InitGoogleLogging + LOG() did not abort");
  int n = 0; LOG_IF(INFO, ++n > 0) << "conditional log";
  QT_CHECK(n == 1, "LOG_IF evaluated its condition once");
  google::ShutdownGoogleLogging();
  (void)argc;
  QT_END();
}
