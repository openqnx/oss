/* libdrm-headers (the kernel DRM UAPI headers): compile a consumer of
 * drm.h, drm_mode.h and drm_fourcc.h and check the values a KMS client
 * puts on the wire -- fourcc codes, format modifiers, ioctl numbers (group
 * 'd', command, argument size in the encoding of the target's
 * <sys/ioccom.h>), and the layout of mode-setting structures. */
#include "common.h"
#include <stddef.h>
#include <stdint.h>
#include <drm.h>
#include <drm_mode.h>
#include <drm_fourcc.h>

int main(void) {
  QT_BEGIN("libdrm-headers");
  QT_CHECK(DRM_FORMAT_XRGB8888 == 0x34325258u, "DRM_FORMAT_XRGB8888 = 'XR24' = 0x34325258");
  QT_CHECK(DRM_FORMAT_ARGB8888 == 0x34325241u && DRM_FORMAT_NV12 == 0x3231564Eu && DRM_FORMAT_YUYV == 0x56595559u,
           "ARGB8888 'AR24', NV12, YUYV fourccs");
  QT_CHECK(DRM_FORMAT_MOD_LINEAR == 0 && DRM_FORMAT_MOD_INVALID == 0x00ffffffffffffffull, "modifiers LINEAR = 0, INVALID");
  QT_CHECK(I915_FORMAT_MOD_X_TILED == ((1ull << 56) | 1) && fourcc_mod_get_vendor(I915_FORMAT_MOD_X_TILED) == DRM_FORMAT_MOD_VENDOR_INTEL,
           "I915_FORMAT_MOD_X_TILED: vendor Intel, code 1");
  QT_CHECK(DRM_FORMAT_BIG_ENDIAN == 0x80000000u, "DRM_FORMAT_BIG_ENDIAN flag");
  /* ioctl numbers: group 'd' in bits 8-15, the command in bits 0-7 */
  unsigned long v = DRM_IOCTL_VERSION, res = DRM_IOCTL_MODE_GETRESOURCES, fb = DRM_IOCTL_MODE_ADDFB2;
  printf("  info DRM_IOCTL_VERSION = 0x%lx, MODE_GETRESOURCES = 0x%lx\n", v, res);
  QT_CHECK(((v >> 8) & 0xff) == 'd' && (v & 0xff) == 0x00, "DRM_IOCTL_VERSION: group 'd', nr 0x00");
  QT_CHECK(((res >> 8) & 0xff) == 'd' && (res & 0xff) == 0xA0 && (fb & 0xff) == 0xB8, "MODE_GETRESOURCES nr 0xA0, MODE_ADDFB2 nr 0xB8");
  QT_CHECK(((res >> 16) & 0x1fff) == sizeof(struct drm_mode_card_res), "ioctl encodes sizeof(struct drm_mode_card_res)");
  QT_CHECK(sizeof(struct drm_mode_modeinfo) == 68 && sizeof(struct drm_mode_card_res) == 64, "modeinfo 68 bytes, card_res 64 bytes");
  QT_CHECK(offsetof(struct drm_mode_fb_cmd2, pixel_format) == 12 && offsetof(struct drm_mode_fb_cmd2, modifier) == 72,
           "drm_mode_fb_cmd2: pixel_format at 12, modifier[] at 72");
  QT_CHECK(DRM_MODE_CONNECTOR_HDMIA == 11 && DRM_MODE_CONNECTOR_eDP == 14 && DRM_MODE_TYPE_PREFERRED == (1 << 3), "connector types, mode flags");
  QT_CHECK(DRM_CAP_DUMB_BUFFER == 0x1 && DRM_CLIENT_CAP_ATOMIC == 3, "DRM_CAP_DUMB_BUFFER, DRM_CLIENT_CAP_ATOMIC");
  struct drm_mode_modeinfo mi; memset(&mi, 0, sizeof mi);
  strncpy(mi.name, "1920x1080", DRM_DISPLAY_MODE_LEN - 1);
  mi.hdisplay = 1920; mi.vdisplay = 1080; mi.clock = 148500; mi.htotal = 2200; mi.vtotal = 1125;
  unsigned refresh = (unsigned)((mi.clock * 1000ull + mi.htotal * mi.vtotal / 2) / ((unsigned long)mi.htotal * mi.vtotal));
  QT_CHECK(refresh == 60 && DRM_DISPLAY_MODE_LEN == 32, "a 1080p60 mode line: refresh 60 Hz");
  QT_END();
}
