/* libmetal (static library, QNX system layer): initialise it (IRQ channel,
 * cache control, generic bus), then use its abstractions the way OpenAMP
 * does: an I/O region over memory with typed atomic accesses, block copies
 * and virtual/physical/offset translation; a shared memory segment opened
 * twice, seen through both mappings; lists and bitmaps; a mutex shared by
 * threads, a condition variable, a spinlock; sleep and the timestamp clock. */
#include "common.h"
#include <metal/sys.h>
#include <metal/io.h>
#include <metal/list.h>
#include <metal/utilities.h>
#include <metal/mutex.h>
#include <metal/condition.h>
#include <metal/spinlock.h>
#include <metal/sleep.h>
#include <metal/time.h>
#include <metal/shmem.h>
#include <metal/alloc.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static metal_mutex_t mu;
static struct metal_condition cv;
static struct metal_spinlock sl = METAL_SPINLOCK_INIT;
static long counter, spun, ready;

static void *worker(void *a) {
  (void)a;
  for (int i = 0; i < 20000; i++) { metal_mutex_acquire(&mu); counter++; metal_mutex_release(&mu); }
  for (int i = 0; i < 20000; i++) { metal_spinlock_acquire(&sl); spun++; metal_spinlock_release(&sl); }
  metal_mutex_acquire(&mu); ready++; metal_condition_signal(&cv); metal_mutex_release(&mu);
  return NULL;
}

struct item { int v; struct metal_list node; };

int main(void) {
  QT_BEGIN("libmetal");
  struct metal_init_params p = METAL_INIT_DEFAULTS;
  p.log_level = METAL_LOG_WARNING;
  int rc = metal_init(&p);
  printf("  info metal_init -> %d\n", rc);
  QT_CHECK(rc == 0, "metal_init (IRQ channel, cache control, generic bus)");
  if (rc) QT_END();

  /* I/O region: 4 KiB of memory, its "physical" base 0x80000000 */
  static unsigned char mem[4096] __attribute__((aligned(64)));
  metal_phys_addr_t phys = 0x80000000u;
  struct metal_io_region io;
  metal_io_init(&io, mem, &phys, sizeof mem, (unsigned)-1, 0, NULL);
  QT_CHECK(metal_io_region_size(&io) == 4096, "I/O region of 4096 bytes");
  metal_io_write8(&io, 0, 0xAB);
  metal_io_write16(&io, 2, 0xBEEF);
  metal_io_write32(&io, 4, 0xDEADBEEFu);
  metal_io_write64(&io, 8, 0x0123456789ABCDEFull);
  QT_CHECK(metal_io_read8(&io, 0) == 0xAB && metal_io_read16(&io, 2) == 0xBEEF && metal_io_read32(&io, 4) == 0xDEADBEEFu
           && metal_io_read64(&io, 8) == 0x0123456789ABCDEFull, "typed writes read back (8/16/32/64 bit)");
  QT_CHECK(mem[4] == 0xEF || mem[4] == 0xDE, "a 32-bit write landed in memory");
  const char msg[] = "remote processor message";
  QT_CHECK(metal_io_block_write(&io, 100, msg, sizeof msg) == (int)sizeof msg, "block_write");
  char back[64] = "";
  QT_CHECK(metal_io_block_read(&io, 100, back, sizeof msg) == (int)sizeof msg && !strcmp(back, msg), "block_read gives it back");
  metal_io_block_set(&io, 200, 0x5A, 32);
  int set = 1; for (int i = 200; i < 232; i++) set &= mem[i] == 0x5A;
  QT_CHECK(set && mem[232] == 0, "block_set fills exactly 32 bytes");
  QT_CHECK(metal_io_virt(&io, 100) == mem + 100 && metal_io_virt_to_offset(&io, mem + 100) == 100, "offset <-> virtual address");
  QT_CHECK(metal_io_phys(&io, 0x10) == 0x80000010u && metal_io_phys_to_offset(&io, 0x80000020u) == 0x20, "offset <-> physical address");
  QT_CHECK(metal_io_phys_to_virt(&io, 0x80000040u) == mem + 0x40 && metal_io_virt_to_phys(&io, mem + 8) == 0x80000008u, "physical <-> virtual");
  QT_CHECK(metal_io_virt(&io, 5000) == NULL && metal_io_virt_to_offset(&io, mem + 5000) == METAL_BAD_OFFSET, "out of range -> NULL / BAD_OFFSET");

  /* shared memory: open twice, each a mapping of the same segment */
  char name[64]; snprintf(name, sizeof name, "/qnxport-metal-%d", (int)getpid());
  struct metal_io_region *s1 = NULL, *s2 = NULL;
  int r1 = metal_shmem_open(name, 8192, &s1), r2 = metal_shmem_open(name, 8192, &s2);
  printf("  info metal_shmem_open -> %d, %d\n", r1, r2);
  QT_CHECK(r1 == 0 && r2 == 0 && s1 && s2 && metal_io_region_size(s1) >= 8192, "metal_shmem_open twice, 8 KiB");
  if (!r1 && !r2) {
    metal_io_write32(s1, 4096, 0xCAFEF00Du);
    metal_io_block_write(s1, 16, "shared", 7);
    char sb[8] = "";
    metal_io_block_read(s2, 16, sb, 7);
    QT_CHECK(metal_io_read32(s2, 4096) == 0xCAFEF00Du && !strcmp(sb, "shared"), "written through one mapping, read through the other");
    metal_io_finish(s1); metal_io_finish(s2);
  }
  shm_unlink(name);

  /* lists and bitmaps */
  struct metal_list head; metal_list_init(&head);
  QT_CHECK(metal_list_is_empty(&head), "new list is empty");
  struct item it[4];
  for (int i = 0; i < 4; i++) { it[i].v = i + 1; metal_list_add_tail(&head, &it[i].node); }
  metal_list_del(&it[1].node);
  struct metal_list *n; int sum = 0, order = 0;
  metal_list_for_each(&head, n) { struct item *x = metal_container_of(n, struct item, node); sum += x->v; order = order * 10 + x->v; }
  QT_CHECK(sum == 8 && order == 134, "list: add_tail 1..4, delete 2 -> 1,3,4");
  QT_CHECK(metal_list_find_node(&head, &it[2].node) && !metal_list_find_node(&head, &it[1].node), "list_find_node");
  unsigned long bm[2] = { 0, 0 };
  metal_bitmap_set_bit(bm, 3); metal_bitmap_set_bit(bm, 70);
  QT_CHECK(metal_bitmap_is_bit_set(bm, 3) && metal_bitmap_is_bit_set(bm, 70) && metal_bitmap_is_bit_clear(bm, 4), "bitmap set/test");
  QT_CHECK(metal_bitmap_next_set_bit(bm, 4, 128) == 70 && metal_bitmap_next_clear_bit(bm, 3, 128) == 4, "bitmap next set/clear bit");
  QT_CHECK(metal_log2(4096) == 12 && metal_align_up(4097, 4096) == 8192 && metal_div_round_up(10, 4) == 3, "log2, align_up, div_round_up");

  /* threads */
  metal_mutex_init(&mu);
  metal_condition_init(&cv);
  pthread_t t[4];
  for (int i = 0; i < 4; i++) pthread_create(&t[i], NULL, worker, NULL);
  metal_mutex_acquire(&mu);
  while (ready < 4) metal_condition_wait(&cv, &mu);
  metal_mutex_release(&mu);
  for (int i = 0; i < 4; i++) pthread_join(t[i], NULL);
  printf("  info counter %ld, spinlock counter %ld\n", counter, spun);
  QT_CHECK(counter == 80000 && spun == 80000, "mutex and spinlock: 4 threads x 20000 increments each");
  QT_CHECK(ready == 4, "condition variable woke the waiter for all 4");
#define OWNER() (((volatile struct _sync_mutex *)&mu)->__owner)   /* QNX's sync_t owner word */
  printf("  info mutex owner word: free 0x%x\n", OWNER());
  QT_CHECK(metal_mutex_try_acquire(&mu), "try_acquire on a free mutex");
  printf("  info mutex owner word: held 0x%x\n", OWNER());
  QT_CHECK(metal_mutex_is_acquired(&mu), "is_acquired while held");
  metal_mutex_release(&mu);
  printf("  info mutex owner word: released 0x%x\n", OWNER());
  QT_CHECK(!metal_mutex_is_acquired(&mu), "not is_acquired once released");
  int again = metal_mutex_try_acquire(&mu);
  QT_CHECK(again, "is_acquired left a free mutex free (try_acquire succeeds)");
  metal_mutex_release(&mu);      /* held either by that try_acquire or by is_acquired's probe */
  metal_mutex_deinit(&mu);

  void *a = metal_allocate_memory(100);
  QT_CHECK(a != NULL, "metal_allocate_memory");
  metal_free_memory(a);

  unsigned long long t0 = metal_get_timestamp();
  QT_CHECK(metal_sleep_usec(20000) == 0, "metal_sleep_usec(20 ms)");
  unsigned long long t1 = metal_get_timestamp();
  printf("  info metal_get_timestamp: %llu -> %llu\n", t0, t1);
  QT_CHECK(t1 > t0 && t1 - t0 >= 15000000ull, "metal_get_timestamp advances (>= 15 ms in ns across the sleep)");
  metal_finish();
  QT_END();
}
