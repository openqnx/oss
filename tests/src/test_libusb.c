/* libusb: context lifecycle and device enumeration. A target with no USB
 * devices is a valid outcome -- enumeration must succeed and return a list. */
#include "common.h"
#include <libusb-1.0/libusb.h>
int main(void) {
  QT_BEGIN("libusb");
  const struct libusb_version *v = libusb_get_version();
  printf("  info libusb %d.%d.%d%s\n", v->major, v->minor, v->micro, v->rc);
  libusb_context *ctx = NULL;
  int rc = libusb_init(&ctx);
  QT_CHECK(rc == 0, "libusb_init");
  if (rc != 0) { printf("  info %s\n", libusb_strerror(rc)); QT_END(); }
  libusb_device **list = NULL;
  ssize_t n = libusb_get_device_list(ctx, &list);
  QT_CHECK(n >= 0, "libusb_get_device_list");
  printf("  info %ld USB device(s) visible\n", (long)n);
  for (ssize_t i = 0; i < n; ++i) {
    struct libusb_device_descriptor d;
    if (libusb_get_device_descriptor(list[i], &d) == 0)
      printf("  info   %04x:%04x bus %u addr %u\n", d.idVendor, d.idProduct,
             libusb_get_bus_number(list[i]), libusb_get_device_address(list[i]));
  }
  if (list) libusb_free_device_list(list, 1);
  QT_CHECK(strcmp(libusb_error_name(LIBUSB_ERROR_NO_DEVICE), "LIBUSB_ERROR_NO_DEVICE") == 0,
           "libusb_error_name");
  QT_CHECK(libusb_has_capability(LIBUSB_CAP_HAS_CAPABILITY) != 0, "capability query");
  libusb_exit(ctx);
  QT_END();
}
