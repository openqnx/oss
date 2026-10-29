/* libfdt: flattened device tree access -- how a QNX BSP reads hardware
 * description blobs. Builds a small DTB in memory and reads it back. */
#include "common.h"
#include <stdlib.h>
#include <libfdt.h>
int main(void) {
  QT_BEGIN("libfdt");
  void *fdt = malloc(4096);
  QT_CHECK(fdt != NULL, "allocate a blob");
  QT_CHECK(fdt_create(fdt, 4096) == 0, "fdt_create");
  fdt_finish_reservemap(fdt);
  fdt_begin_node(fdt, "");
  fdt_property_string(fdt, "compatible", "qnx,port-test");
  fdt_property_u32(fdt, "#address-cells", 1);
  fdt_begin_node(fdt, "cpus");
  fdt_begin_node(fdt, "cpu@0");
  fdt_property_string(fdt, "device_type", "cpu");
  fdt_property_u32(fdt, "reg", 0);
  fdt_end_node(fdt);
  fdt_end_node(fdt);
  fdt_end_node(fdt);
  QT_CHECK(fdt_finish(fdt) == 0, "fdt_finish");

  QT_CHECK(fdt_check_header(fdt) == 0, "fdt_check_header");
  QT_CHECK(fdt_magic(fdt) == FDT_MAGIC, "magic number");
  int root = 0;
  const char *compat = fdt_getprop(fdt, root, "compatible", NULL);
  QT_CHECK(compat && strcmp(compat, "qnx,port-test") == 0, "root compatible string");
  int cpus = fdt_path_offset(fdt, "/cpus");
  QT_CHECK(cpus >= 0, "fdt_path_offset(/cpus)");
  int cpu0 = fdt_path_offset(fdt, "/cpus/cpu@0");
  QT_CHECK(cpu0 >= 0, "fdt_path_offset(/cpus/cpu@0)");
  int len = 0;
  const fdt32_t *reg = fdt_getprop(fdt, cpu0, "reg", &len);
  QT_CHECK(reg && len == 4 && fdt32_to_cpu(*reg) == 0, "reg property (byte order handled)");
  const char *dt = fdt_getprop(fdt, cpu0, "device_type", NULL);
  QT_CHECK(dt && strcmp(dt, "cpu") == 0, "device_type");
  QT_CHECK(fdt_path_offset(fdt, "/nope") == -FDT_ERR_NOTFOUND, "missing path reports NOTFOUND");
  QT_CHECK(strcmp(fdt_strerror(-FDT_ERR_NOTFOUND), "FDT_ERR_NOTFOUND") == 0, "fdt_strerror");
  free(fdt);
  QT_END();
}
