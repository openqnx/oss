#include "common.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <hailo/hailort.h>
/* HailoRT without a Hailo device (none on the test targets): the library
 * reports its version (4.14.0); the device scan finds no device and says so
 * instead of failing; creating a virtual device then fails cleanly with
 * HAILO_OUT_OF_PHYSICAL_DEVICES. HEF parsing, with a HEF the test makes: a
 * header (magic "\x01HEF", version 0, length, MD5 of the body) and a 12-byte
 * protobuf body (a Hailo-8L header, SDK version "3.24.0", no network groups)
 * parses from memory and from a file and lists no network groups; one
 * flipped body byte (the MD5 no longer matches) and a wrong magic are
 * rejected with HAILO_INVALID_HEF; a missing file with
 * HAILO_OPEN_FILE_FAILURE. hailortcli --version names the same version. */
static const uint8_t body[12] = { 0x0a, 0x0a, 0x08, 0x03, 0x1a, 0x06, '3', '.', '2', '4', '.', '0' };
static const uint8_t body_md5[16] = { 0x3f, 0xc5, 0xa7, 0x4f, 0xfb, 0xea, 0xf7, 0xe5,
                                      0x28, 0x32, 0x0b, 0x38, 0xa6, 0x12, 0x16, 0x6a };
static size_t make_hef(uint8_t *out) {
  const uint8_t hdr[16] = { 0x01, 'H', 'E', 'F', 0, 0, 0, 0, 0, 0, 0, sizeof body, 0, 0, 0, 0 };
  memcpy(out, hdr, 16); memcpy(out + 16, body_md5, 16); memcpy(out + 32, body, sizeof body);
  return 32 + sizeof body;
}
int main(void) {
  QT_BEGIN("hailort");
  /* the library's log file goes to $TMPDIR, not the current directory */
  const char *tmp = getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp";
  setenv("HAILORT_LOGGER_PATH", tmp, 1);
  hailo_version_t v = {0};
  QT_CHECK(hailo_get_library_version(&v) == HAILO_SUCCESS && v.major == 4 && v.minor == 14 && v.revision == 0,
           "library version 4.14.0");

  hailo_device_id_t ids[8]; size_t n = 8;
  hailo_status st = hailo_scan_devices(NULL, ids, &n);
  printf("  info scan: status %d, %zu device(s)\n", (int)st, n);
  QT_CHECK(st == HAILO_SUCCESS && n == 0, "device scan: no Hailo device found, no error");
  hailo_vdevice vd = NULL;
  st = hailo_create_vdevice(NULL, &vd);
  printf("  info create_vdevice: status %d\n", (int)st);
  QT_CHECK(st != HAILO_SUCCESS && vd == NULL, "virtual device refused without hardware");
  QT_CHECK(st == HAILO_OUT_OF_PHYSICAL_DEVICES, "... with HAILO_OUT_OF_PHYSICAL_DEVICES");

  uint8_t hef[64]; size_t len = make_hef(hef);
  hailo_hef h = NULL;
  st = hailo_create_hef_buffer(&h, hef, len);
  QT_CHECK(st == HAILO_SUCCESS && h != NULL, "HEF parsed from memory (header, MD5, protobuf body)");
  if (h) {
    hailo_network_group_info_t infos[4]; size_t ni = 4;
    st = hailo_get_network_groups_infos(h, infos, &ni);
    QT_CHECK(st == HAILO_SUCCESS && ni == 0, "the HEF has no network groups");
    hailo_release_hef(h); h = NULL;
  }
  char path[256]; snprintf(path, sizeof path, "%s/test_hailort_%d.hef", tmp, (int)getpid());
  FILE *f = fopen(path, "wb"); if (f) { fwrite(hef, 1, len, f); fclose(f); }
  st = hailo_create_hef_file(&h, path);
  QT_CHECK(st == HAILO_SUCCESS && h != NULL, "HEF parsed from a file");
  if (h) hailo_release_hef(h), h = NULL;
  unlink(path);

  uint8_t bad[64]; memcpy(bad, hef, len); bad[40] ^= 1;
  QT_CHECK(hailo_create_hef_buffer(&h, bad, len) == HAILO_INVALID_HEF, "damaged body (MD5 mismatch): HAILO_INVALID_HEF");
  memcpy(bad, hef, len); bad[1] = 'X';
  QT_CHECK(hailo_create_hef_buffer(&h, bad, len) == HAILO_INVALID_HEF, "wrong magic: HAILO_INVALID_HEF");
  QT_CHECK(hailo_create_hef_file(&h, "/nonexistent/model.hef") == HAILO_OPEN_FILE_FAILURE, "missing file: HAILO_OPEN_FILE_FAILURE");

  FILE *p = popen("hailortcli --version 2>&1", "r");
  char line[256] = "";
  if (p) { if (!fgets(line, sizeof line, p)) line[0] = 0; pclose(p); }
  line[strcspn(line, "\n")] = 0;
  printf("  info %s\n", line);
  QT_CHECK(strstr(line, "HailoRT-CLI version 4.14.0") != NULL, "hailortcli --version: 4.14.0");
  p = popen("hailortcli scan 2>&1", "r");
  int found = 0;
  if (p) { while (fgets(line, sizeof line, p)) if (strstr(line, "Hailo devices not found")) found = 1; pclose(p); }
  QT_CHECK(found, "hailortcli scan: \"Hailo devices not found\"");
  QT_END();
}
