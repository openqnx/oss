/* libmodbus: TCP server and client on loopback, read back registers. */
#include "common.h"
#include <modbus.h>
#include <pthread.h>
#include <unistd.h>
static modbus_mapping_t *map;
static void *serve(void *x) {
  modbus_t *s = modbus_new_tcp("127.0.0.1", 15020); int ls = modbus_tcp_listen(s, 1);
  modbus_tcp_accept(s, &ls);
  uint8_t q[MODBUS_TCP_MAX_ADU_LENGTH]; int n;
  while ((n = modbus_receive(s, q)) > 0) modbus_reply(s, q, n, map);
  close(ls); modbus_free(s); return NULL;
}
int main(void) {
  QT_BEGIN("libmodbus");
  map = modbus_mapping_new(0, 0, 10, 0); map->tab_registers[3] = 0x1234;
  pthread_t t; pthread_create(&t, NULL, serve, NULL);
  modbus_t *c = modbus_new_tcp("127.0.0.1", 15020); int rc = -1;
  for (int i = 0; i < 50 && rc; i++) { rc = modbus_connect(c); if (rc) usleep(100000); }
  QT_CHECK(rc == 0, "client connects");
  uint16_t r[4] = {0};
  QT_CHECK(modbus_read_registers(c, 0, 4, r) == 4, "read 4 holding registers");
  QT_CHECK(r[3] == 0x1234, "register value");
  QT_CHECK(modbus_write_register(c, 1, 77) == 1 && map->tab_registers[1] == 77, "write register");
  modbus_close(c); modbus_free(c); pthread_join(t, NULL); modbus_mapping_free(map);
  QT_END();
}
