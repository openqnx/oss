/* libretro-retro8: the retro8 PICO-8 emulator as a libretro core, given a
 * one-line cart (cls(8) in _draw): after 120 frames the screen must be
 * PICO-8 red. The frontend is test_libretro_samples.c. */
#define CORE_TEST_NAME "libretro-retro8"
#define CORE_FILE "retro8_libretro_qnx.so"
#define CORE_LIBNAME "retro-8"
#define CORE_FRAMES 120
#define CORE_EXPECT_AUDIO 1
#define CORE_EXPECT_RED 1
#define CORE_CART "pico-8 cartridge // http://www.pico-8.com\nversion 18\n__lua__\nfunction _draw()\n cls(8)\nend\n"
#include "test_libretro_samples.c"
