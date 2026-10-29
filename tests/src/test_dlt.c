#include "common.h"
#include <dlt/dlt.h>
/* dlt-daemon need not be running: registration falls back to local buffering
 * and the API must still behave. */
DLT_DECLARE_CONTEXT(ctx)
int main(void) {
  QT_BEGIN("dlt-daemon");
  DLT_REGISTER_APP("QTST", "QNX port test");
  DLT_REGISTER_CONTEXT(ctx, "MAIN", "main context");
  QT_CHECK(1, "app + context registration returned");
  DLT_LOG(ctx, DLT_LOG_INFO, DLT_STRING("dlt log line from QNX"), DLT_INT32(4660));
  DLT_LOG(ctx, DLT_LOG_WARN, DLT_STRING("second record"));
  QT_CHECK(1, "DLT_LOG did not abort");
  DLT_UNREGISTER_CONTEXT(ctx);
  DLT_UNREGISTER_APP();
  QT_CHECK(1, "clean unregister");
  QT_END();
}
