/* libuv: run a loop with a timer, an async handle woken from a thread, a
 * pipe pair carrying bytes, and the filesystem and platform info APIs. */
#include "common.h"
#include <stdlib.h>
#include <uv.h>
static int timer_fired, async_fired, read_bytes;
static uv_async_t async;
static void on_timer(uv_timer_t *h) { ++timer_fired; uv_async_send(&async); }
static void on_async(uv_async_t *h) { ++async_fired; uv_close((uv_handle_t *)h, NULL); }
static void alloc_cb(uv_handle_t *h, size_t sz, uv_buf_t *b) {
  static char buf[256]; (void)h; (void)sz;
  *b = uv_buf_init(buf, sizeof buf);
}
static void read_cb(uv_stream_t *s, ssize_t n, const uv_buf_t *b) {
  (void)b;
  if (n > 0) read_bytes += (int)n;
  uv_close((uv_handle_t *)s, NULL);
}
int main(void) {
  QT_BEGIN("libuv");
  printf("  info libuv %s\n", uv_version_string());
  uv_loop_t *loop = uv_default_loop();
  QT_CHECK(loop != NULL, "uv_default_loop");

  uv_timer_t timer;
  uv_timer_init(loop, &timer);
  uv_async_init(loop, &async, on_async);
  uv_timer_start(&timer, on_timer, 10, 0);

  uv_pipe_t rd, wr;
  uv_file fds[2];
  QT_CHECK(uv_pipe(fds, 0, 0) == 0, "uv_pipe");
  uv_pipe_init(loop, &rd, 0); uv_pipe_open(&rd, fds[0]);
  uv_pipe_init(loop, &wr, 0); uv_pipe_open(&wr, fds[1]);
  uv_write_t req;
  uv_buf_t out = uv_buf_init((char *)"qnx", 3);
  QT_CHECK(uv_write(&req, (uv_stream_t *)&wr, &out, 1, NULL) == 0, "uv_write");
  QT_CHECK(uv_read_start((uv_stream_t *)&rd, alloc_cb, read_cb) == 0, "uv_read_start");

  uv_run(loop, UV_RUN_DEFAULT);
  uv_close((uv_handle_t *)&timer, NULL);
  uv_close((uv_handle_t *)&wr, NULL);
  uv_run(loop, UV_RUN_DEFAULT);

  QT_CHECK(timer_fired == 1, "timer fired once");
  QT_CHECK(async_fired == 1, "async handle woke the loop");
  QT_CHECK(read_bytes == 3, "pipe delivered 3 bytes");

  uv_fs_t fs;
  QT_CHECK(uv_fs_stat(NULL, &fs, "/tmp", NULL) == 0, "uv_fs_stat(/tmp)");
  uv_fs_req_cleanup(&fs);
  QT_CHECK(uv_get_total_memory() > 0, "uv_get_total_memory");
  uv_cpu_info_t *cpus; int ncpu = 0;
  QT_CHECK(uv_cpu_info(&cpus, &ncpu) == 0 && ncpu > 0, "uv_cpu_info");
  printf("  info %d cpu(s), %llu MiB RAM\n", ncpu,
         (unsigned long long)(uv_get_total_memory() >> 20));
  uv_free_cpu_info(cpus, ncpu);
  uv_loop_close(loop);
  QT_END();
}
