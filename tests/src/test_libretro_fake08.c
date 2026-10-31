/* libretro-fake08: the fake-08 PICO-8 emulator as a libretro core, given a
 * one-line cart (cls(8) in _draw) written to /tmp: after 120 frames the
 * screen must be PICO-8 red. The frontend is test_libretro_samples.c. */
#define CORE_TEST_NAME "libretro-fake08"
#define CORE_FILE "fake08_libretro_qnx.so"
#define CORE_LIBNAME "fake-08"
#define CORE_FRAMES 120
#define CORE_EXPECT_AUDIO 1
#define CORE_EXPECT_RED 1
#define CORE_CART "pico-8 cartridge // http://www.pico-8.com\nversion 18\n__lua__\nfunction _draw()\n cls(8)\nend\n"
#include "test_libretro_samples.c"
