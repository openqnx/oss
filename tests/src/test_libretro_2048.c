/* libretro-2048: the 2048 puzzle as a libretro core, started without content
 * (it renders its own board). The frontend is test_libretro_samples.c. */
#define CORE_TEST_NAME "libretro-2048"
#define CORE_FILE "2048_libretro_qnx.so"
#define CORE_LIBNAME "2048"
#define CORE_FRAMES 120
#define CORE_EXPECT_AUDIO 0          /* 2048 is silent */
#define CORE_CHECK_COLOURS 3
#include "test_libretro_samples.c"
