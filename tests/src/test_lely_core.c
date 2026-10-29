#include "common.h"
#include <lely/co/dev.h>
#include <lely/co/obj.h>
/* Builds a CANopen device object dictionary in memory -- no CAN hardware. */
int main(void) {
  QT_BEGIN("lely-core");
  co_dev_t *dev = co_dev_create(0x01);
  QT_CHECK(dev != NULL, "co_dev_create (node id 1)");
  if (!dev) QT_END();
  QT_CHECK(co_dev_get_id(dev) == 0x01, "node id readback");
  co_obj_t *obj = co_obj_create(0x1000);   /* device type, mandatory */
  QT_CHECK(obj != NULL, "co_obj_create 0x1000");
  QT_CHECK(obj && co_dev_insert_obj(dev, obj) == 0, "insert object into dictionary");
  QT_CHECK(co_dev_find_obj(dev, 0x1000) == obj, "find object by index");
  co_sub_t *sub = co_sub_create(0x00, CO_DEFTYPE_UNSIGNED32);
  QT_CHECK(sub != NULL, "co_sub_create subindex 0");
  QT_CHECK(sub && co_obj_insert_sub(obj, sub) == 0, "insert sub-object");
  co_dev_destroy(dev);
  QT_END();
}
