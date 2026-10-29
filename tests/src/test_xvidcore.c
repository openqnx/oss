/* xvidcore: the MPEG-4 ASP codec. Library init detects CPU features, which is
 * the part most likely to break on a new architecture, and the info query
 * reports back what the build actually produced. */
#include "common.h"
#include <string.h>
#include <xvid.h>
int main(void) {
  QT_BEGIN("xvidcore");
  xvid_gbl_init_t init;
  memset(&init, 0, sizeof init);
  init.version = XVID_VERSION;
  init.cpu_flags = 0;   /* autodetect */
  QT_CHECK(xvid_global(NULL, XVID_GBL_INIT, &init, NULL) >= 0, "xvid_global(XVID_GBL_INIT)");

  xvid_gbl_info_t info;
  memset(&info, 0, sizeof info);
  info.version = XVID_VERSION;
  QT_CHECK(xvid_global(NULL, XVID_GBL_INFO, &info, NULL) >= 0, "xvid_global(XVID_GBL_INFO)");
  printf("  info xvidcore %d.%d.%d, %d thread(s), build \"%s\"\n",
         XVID_VERSION_MAJOR(info.actual_version),
         XVID_VERSION_MINOR(info.actual_version),
         XVID_VERSION_PATCH(info.actual_version),
         info.num_threads, info.build ? info.build : "?");
  QT_CHECK(XVID_VERSION_MAJOR(info.actual_version) == 1, "reports version 1.x");
  QT_CHECK(info.num_threads >= 1, "detected at least one cpu");
  QT_END();
}
