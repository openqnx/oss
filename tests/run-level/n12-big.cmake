# Run-level tests, group n12-big: Mesa as a software renderer (llvmpipe,
# softpipe, lavapipe) in its own prefix, /usr/local/opt/mesa, beside the
# SDP's libEGL/libGLESv2 for QNX Screen. Off-screen GL/GLES/Vulkan with known
# pixels and buffers (test_mesa), and a Wayland client presenting through
# wl_shm to a compositor inside the test (test_mesa_wayland). Which
# component each test checks: n12-big.tsv; what the tests need installed
# beyond what they link: n12-big.extras.
set(_mesa "${STAGE}/opt/mesa")
# Mesa's headers first: the SDP's EGL/GLES headers are on the default path.
port_test(test_mesa          test_mesa.c
          "${_mesa}/lib/libEGL_mesa.so" "${_mesa}/lib/libGLESv2_mesa.so")
target_include_directories(test_mesa BEFORE PRIVATE "${_mesa}/include")
target_link_options(test_mesa PRIVATE "-Wl,-rpath-link,${_mesa}/lib" "-Wl,-rpath,/usr/local/opt/mesa/lib")
port_test(test_mesa_wayland  test_mesa_wayland.c
          "${_mesa}/lib/libEGL_mesa.so" "${_mesa}/lib/libGLESv2_mesa.so"
          wayland-server wayland-client wayland-egl)
target_include_directories(test_mesa_wayland BEFORE PRIVATE "${_mesa}/include")
target_link_options(test_mesa_wayland PRIVATE "-Wl,-rpath-link,${_mesa}/lib" "-Wl,-rpath,/usr/local/opt/mesa/lib")
