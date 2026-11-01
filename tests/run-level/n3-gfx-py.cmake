# Run-level tests, group n3-gfx-py: graphics libraries (glfw, raylib,
# SDL_mixer, libraqm, Qhull), Python packages (pygame, matplotlib and its
# dependencies, Pillow, ml_dtypes, psutil, colcon and its dependencies),
# QDirStat and xtl. The target has no display: windowing code runs on the
# null/offscreen/dummy backends or a headless labwc. Which component each
# test checks: n3-gfx-py.tsv; what the tests need installed beyond what they
# link: n3-gfx-py.extras.
find_package(PkgConfig REQUIRED)
pkg_check_modules(N3_RAQM REQUIRED IMPORTED_TARGET raqm freetype2)

port_test(test_xtl                test_xtl.cpp)
port_test(test_glfw               test_glfw.c               glfw)
port_test(test_raylib             test_raylib.c             raylib m)
# the sdl2 .pc file requires alsa (no .pc in the SDP): link by name
port_test(test_sdl2_mixer         test_sdl2_mixer.c         SDL2_mixer SDL2 m)
target_include_directories(test_sdl2_mixer PRIVATE "${INC}/SDL2")
port_test(test_libraqm            test_libraqm.c            PkgConfig::N3_RAQM)
port_test(test_qhull              test_qhull.c              qhull_r m)

port_script(test_psutil)
port_script(test_python3_colcon_common_extensions)
port_script(test_python3_ml_dtypes)
port_script(test_python3_matplotlib)
port_script(test_python3_pillow)
port_script(test_pygame)
port_script(test_qdirstat)
