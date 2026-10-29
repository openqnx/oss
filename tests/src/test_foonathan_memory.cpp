#include "common.h"
#include <foonathan/memory/memory_pool.hpp>
#include <foonathan/memory/allocator_storage.hpp>
int main() {
  QT_BEGIN("foonathan_memory");
  namespace fm = foonathan::memory;
  fm::memory_pool<> pool(32, 4096);
  void *a = pool.allocate_node();
  void *b = pool.allocate_node();
  QT_CHECK(a != NULL && b != NULL && a != b, "two distinct nodes from pool");
  memset(a, 0xAB, 32);
  QT_CHECK(((unsigned char *)a)[31] == 0xAB, "node is writable for its full size");
  pool.deallocate_node(a);
  pool.deallocate_node(b);
  QT_CHECK(pool.node_size() >= 32, "node_size honours request");
  QT_END();
}
