/* libretro-mrboom: Mr.Boom (Bomberman clone) as a libretro core, started
 * without content; runs its intro for 300 frames with music. The frontend is
 * test_libretro_samples.c. */
#define CORE_TEST_NAME "libretro-mrboom"
#define CORE_FILE "mrboom_libretro_qnx.so"
#define CORE_LIBNAME "Mr.Boom"
#define CORE_FRAMES 300
#define CORE_EXPECT_AUDIO 1
#define CORE_CHECK_COLOURS 2
#include "test_libretro_samples.c"
