/* rtl-sdr without a dongle (the QEMU target has no USB radio): the library
 * enumerates through libusb and must report "no devices" cleanly on every
 * entry point, and the programs must say so instead of crashing. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <rtl-sdr.h>

static int run_capture(const char *cmd, char *out, size_t n) {
  char full[512];
  snprintf(full, sizeof full, "%s 2>&1", cmd);
  FILE *p = popen(full, "r");
  if (!p) return -1;
  size_t k = fread(out, 1, n - 1, p); out[k] = 0;
  int st = pclose(p);
  printf("  info $ %s -> status 0x%x\n", cmd, st);
  const char *q = out;
  for (int i = 0; i < 4 && *q; i++) {
    const char *e = strchr(q, '\n'); int len = e ? (int)(e - q) : (int)strlen(q);
    printf("  info   %.*s\n", len, q);
    q = e ? e + 1 : q + len;
  }
  return st;
}

int main(void) {
  QT_BEGIN("rtl-sdr");
  uint32_t n = rtlsdr_get_device_count();
  printf("  info rtlsdr_get_device_count() = %u\n", n);
  QT_CHECK(n == 0, "no RTL2832 devices on this target");
  const char *name = rtlsdr_get_device_name(0);
  QT_CHECK(name != NULL && name[0] == 0, "rtlsdr_get_device_name(0) is \"\"");
  char m[256] = "", p[256] = "", s[256] = "";
  /* Upstream 2.0.2 overwrites its "-2 = not found" result with libusb_init()'s
     0, so with no dongle it returns 0 (also on Linux) -- but it must not
     invent any strings. */
  int us = rtlsdr_get_device_usb_strings(0, m, p, s);
  printf("  info rtlsdr_get_device_usb_strings(0) = %d\n", us);
  QT_CHECK(us <= 0 && m[0] == 0 && p[0] == 0 && s[0] == 0,
           "rtlsdr_get_device_usb_strings(0): no strings for a missing device");
  QT_CHECK(rtlsdr_get_index_by_serial(NULL) == -1, "get_index_by_serial(NULL) = -1");
  QT_CHECK(rtlsdr_get_index_by_serial("00000001") == -2, "get_index_by_serial: -2 (no devices)");
  rtlsdr_dev_t *dev = NULL;
  int rc = rtlsdr_open(&dev, 0);
  printf("  info rtlsdr_open(0) = %d\n", rc);
  QT_CHECK(rc < 0 && dev == NULL, "rtlsdr_open(0) fails without a device");
  QT_CHECK(rtlsdr_close(NULL) < 0, "rtlsdr_close(NULL) is rejected");
  QT_CHECK(rtlsdr_set_center_freq(NULL, 100000000) < 0, "set_center_freq(NULL) is rejected");

  static char out[8192];
  int st = run_capture("rtl_test -t", out, sizeof out);
  QT_CHECK(strstr(out, "No supported devices found.") != NULL, "rtl_test: \"No supported devices found.\"");
  QT_CHECK(st != -1 && WEXITSTATUS(st) == 1, "rtl_test exits 1");
  st = run_capture("rtl_eeprom", out, sizeof out);
  QT_CHECK(strstr(out, "No supported devices found.") != NULL, "rtl_eeprom: \"No supported devices found.\"");
  st = run_capture("rtl_sdr -h", out, sizeof out);
  QT_CHECK(strstr(out, "Usage:") != NULL && strstr(out, "rtl_sdr"), "rtl_sdr -h prints its usage");
  st = run_capture("rtl_fm -d 0 -f 100M /dev/null", out, sizeof out);
  QT_CHECK(strstr(out, "No supported devices found.") != NULL, "rtl_fm: \"No supported devices found.\"");
  QT_END();
}
