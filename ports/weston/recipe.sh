VERSION=13.0.3; GIT=https://gitlab.freedesktop.org/wayland/weston.git; REF=13.0.3
STATUS=blocked
# Weston hard-requires libinput and libdrm at configure time even when only the
# headless backend is selected (meson.build:139-143 declares both as plain
# dependency() calls, and compositor/meson.build links dep_libinput into the
# weston binary itself). libinput in turn needs udev, libevdev and mtdev -- the
# Linux device model, which QNX does not have; QNX enumerates input through its
# own devi-* resource managers.
#
# Making this work means patching weston to treat libinput/libevdev/libdrm as
# optional for non-DRM backends, and porting libdrm headers-only for the
# drm_fourcc.h pixel format codes weston uses everywhere. That is a project in
# itself, not a port.
#
# GTK3's widget layer is instead exercised through the Broadway backend, which
# needs no compositor at all -- see ports/gtk3 and tests/src/test_gtk3.c.
