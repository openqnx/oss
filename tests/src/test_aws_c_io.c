/* aws-c-io: the event loop (tasks now/in the future, on the loop thread), a
 * pipe driven by the loop, a TCP ping/pong between an aws_socket listener and
 * client on 127.0.0.1, input streams and PEM parsing. */
#include "common.h"
#include <unistd.h>
#include <aws/common/common.h>
#include <aws/common/clock.h>
#include <aws/common/condition_variable.h>
#include <aws/common/mutex.h>
#include <aws/common/task_scheduler.h>
#include <aws/io/io.h>
#include <aws/io/event_loop.h>
#include <aws/io/pipe.h>
#include <aws/io/socket.h>
#include <aws/io/stream.h>
#include <aws/io/pem.h>

static struct aws_allocator *al;
static struct aws_mutex mtx = AWS_MUTEX_INIT;
static struct aws_condition_variable cv = AWS_CONDITION_VARIABLE_INIT;
static struct aws_event_loop *loop;
static int flags;                /* bits set by callbacks, read under mtx */
enum { F_NOW = 1, F_FUTURE = 2, F_ORDER_OK = 4, F_PIPE = 8, F_PONG = 16, F_PING = 32, F_CLOSED = 64, F_ONLOOP = 128 };
static void set_flag(int f) { aws_mutex_lock(&mtx); flags |= f; aws_condition_variable_notify_all(&cv); aws_mutex_unlock(&mtx); }
static int want_flags;
static bool have_flags(void *ud) { (void)ud; return (flags & want_flags) == want_flags; }
static int wait_flags(int f) {
  aws_mutex_lock(&mtx);
  want_flags = f;
  aws_condition_variable_wait_for_pred(&cv, &mtx, 10LL * 1000000000LL, have_flags, NULL);
  int ok = (flags & f) == f;
  aws_mutex_unlock(&mtx);
  return ok;
}

/* ---- tasks ---- */
static void task_now(struct aws_task *t, void *ud, enum aws_task_status st) {
  (void)t; (void)ud;
  if (st == AWS_TASK_STATUS_RUN_READY && aws_event_loop_thread_is_callers_thread(loop)) set_flag(F_NOW | F_ONLOOP);
}
static void task_future(struct aws_task *t, void *ud, enum aws_task_status st) {
  (void)t; (void)ud;
  if (st != AWS_TASK_STATUS_RUN_READY) return;
  aws_mutex_lock(&mtx); int now_ran = flags & F_NOW; aws_mutex_unlock(&mtx);
  set_flag(F_FUTURE | (now_ran ? F_ORDER_OK : 0));
}

/* ---- pipe ---- */
static struct aws_pipe_read_end rend;
static struct aws_pipe_write_end wend;
static char pipe_got[64]; static size_t pipe_len;
static void pipe_readable(struct aws_pipe_read_end *re, int err, void *ud) {
  (void)ud;
  if (err) return;
  struct aws_byte_buf b = aws_byte_buf_from_empty_array(pipe_got + pipe_len, sizeof pipe_got - 1 - pipe_len);
  size_t n = 0;
  while (aws_pipe_read(re, &b, &n) == AWS_OP_SUCCESS && n > 0) pipe_len += n;
  if (pipe_len >= 13 && memcmp(pipe_got, "through pipes", 13) == 0) set_flag(F_PIPE);
}
static void pipe_wrote(struct aws_pipe_write_end *w, int err, struct aws_byte_cursor c, void *ud) { (void)w; (void)err; (void)c; (void)ud; }
static void pipe_start(struct aws_task *t, void *ud, enum aws_task_status st) {
  (void)t; (void)ud; (void)st;
  aws_pipe_subscribe_to_readable_events(&rend, pipe_readable, NULL);
  aws_pipe_write(&wend, aws_byte_cursor_from_c_str("through pipes"), pipe_wrote, NULL);
}
static void pipe_stop(struct aws_task *t, void *ud, enum aws_task_status st) {
  (void)t; (void)ud; (void)st;
  aws_pipe_unsubscribe_from_readable_events(&rend);
  aws_pipe_clean_up_read_end(&rend);
  aws_pipe_clean_up_write_end(&wend);
  set_flag(F_CLOSED);
}

/* ---- sockets ---- */
static struct aws_socket listener, client, *accepted;
static void read_all(struct aws_socket *s, char *dst, size_t cap, size_t *len) {
  struct aws_byte_buf b = aws_byte_buf_from_empty_array(dst + *len, cap - 1 - *len);
  size_t n = 0;
  while (aws_socket_read(s, &b, &n) == AWS_OP_SUCCESS && n > 0) *len += n;
}
/* aws_socket_write requires a completion callback */
static void wrote(struct aws_socket *s, int err, size_t n, void *ud) { (void)s; (void)err; (void)n; (void)ud; }
static char srv_buf[32], cli_buf[32]; static size_t srv_len, cli_len;
static void server_readable(struct aws_socket *s, int err, void *ud) {
  (void)ud;
  if (err) return;
  read_all(s, srv_buf, sizeof srv_buf, &srv_len);
  if (srv_len == 4 && memcmp(srv_buf, "ping", 4) == 0) {
    set_flag(F_PING);
    struct aws_byte_cursor pong = aws_byte_cursor_from_c_str("pong");
    aws_socket_write(s, &pong, wrote, NULL);
  }
}
static void on_accept(struct aws_socket *l, int err, struct aws_socket *ns, void *ud) {
  (void)l; (void)ud;
  if (err || !ns) { printf("  info accept error %d\n", err); return; }
  accepted = ns;
  aws_socket_assign_to_event_loop(ns, loop);
  aws_socket_subscribe_to_readable_events(ns, server_readable, NULL);
}
static void client_readable(struct aws_socket *s, int err, void *ud) {
  (void)ud;
  if (err) return;
  read_all(s, cli_buf, sizeof cli_buf, &cli_len);
  if (cli_len == 4 && memcmp(cli_buf, "pong", 4) == 0) set_flag(F_PONG);
}
static void on_connect(struct aws_socket *s, int err, void *ud) {
  (void)ud;
  if (err) { printf("  info connect error %d (%s)\n", err, aws_error_name(err)); return; }
  aws_socket_subscribe_to_readable_events(s, client_readable, NULL);
  struct aws_byte_cursor ping = aws_byte_cursor_from_c_str("ping");
  aws_socket_write(s, &ping, wrote, NULL);
}
static void sockets_close(struct aws_task *t, void *ud, enum aws_task_status st) {
  (void)t; (void)ud; (void)st;
  aws_socket_close(&client);
  if (accepted) aws_socket_close(accepted);
  aws_socket_stop_accept(&listener);
  aws_socket_close(&listener);
  set_flag(F_CLOSED);
}

static const char *pem_text =
  "-----BEGIN CERTIFICATE-----\nAAECAwQFBgcICQoLDA0ODw==\n-----END CERTIFICATE-----\n"
  "junk between objects\n"
  "-----BEGIN PRIVATE KEY-----\nSGVsbG8=\n-----END PRIVATE KEY-----\n";

int main(void) {
  QT_BEGIN("aws-c-io");
  alarm(60);
  al = aws_default_allocator();
  aws_io_library_init(al);

  struct aws_event_loop_group_options eo = {0};
  eo.loop_count = 1;
  struct aws_event_loop_group *elg = aws_event_loop_group_new(al, &eo);
  QT_CHECK(elg != NULL, "event loop group with one loop");
  if (!elg) QT_END();
  loop = aws_event_loop_group_get_next_loop(elg);

  struct aws_task tnow, tfut;
  aws_task_init(&tfut, task_future, NULL, "future");
  aws_task_init(&tnow, task_now, NULL, "now");
  uint64_t t0 = 0;
  aws_event_loop_current_clock_time(loop, &t0);
  aws_event_loop_schedule_task_future(loop, &tfut, t0 + 20 * 1000000ULL);
  aws_event_loop_schedule_task_now(loop, &tnow);
  QT_CHECK(wait_flags(F_NOW | F_FUTURE), "immediate and 20 ms tasks both ran");
  QT_CHECK(flags & F_ONLOOP, "tasks run on the event loop thread");
  QT_CHECK(flags & F_ORDER_OK, "the immediate task ran before the delayed one");

  QT_CHECK(aws_pipe_init(&rend, loop, &wend, loop, al) == AWS_OP_SUCCESS, "pipe created on the loop");
  struct aws_task tp, tps;
  aws_task_init(&tp, pipe_start, NULL, "pipe");
  aws_event_loop_schedule_task_now(loop, &tp);
  QT_CHECK(wait_flags(F_PIPE), "bytes written to the pipe arrive through the readable callback");
  aws_task_init(&tps, pipe_stop, NULL, "pipe-stop");
  aws_event_loop_schedule_task_now(loop, &tps);
  wait_flags(F_CLOSED);
  flags &= ~F_CLOSED;

  struct aws_socket_options so = {0};
  so.type = AWS_SOCKET_STREAM; so.domain = AWS_SOCKET_IPV4; so.connect_timeout_ms = 5000;
  QT_CHECK(aws_socket_init(&listener, al, &so) == AWS_OP_SUCCESS, "listener socket created");
  struct aws_socket_endpoint ep = {0};
  strcpy(ep.address, "127.0.0.1"); ep.port = 0;
  struct aws_socket_bind_options bo = {0};
  bo.local_endpoint = &ep;
  int bound = aws_socket_bind(&listener, &bo) == AWS_OP_SUCCESS;
  struct aws_socket_endpoint got = {0};
  bound = bound && aws_socket_get_bound_address(&listener, &got) == AWS_OP_SUCCESS && got.port > 0;
  printf("  info listening on 127.0.0.1:%u\n", (unsigned)got.port);
  QT_CHECK(bound && aws_socket_listen(&listener, 8) == AWS_OP_SUCCESS, "bind to an ephemeral loopback port and listen");
  struct aws_socket_listener_options lo = {0};
  lo.on_accept_result = on_accept;
  QT_CHECK(aws_socket_start_accept(&listener, loop, lo) == AWS_OP_SUCCESS, "accepting on the event loop");
  aws_socket_init(&client, al, &so);
  struct aws_socket_connect_options co = {0};
  co.remote_endpoint = &got; co.event_loop = loop; co.on_connection_result = on_connect;
  QT_CHECK(aws_socket_connect(&client, &co) == AWS_OP_SUCCESS, "connect started");
  QT_CHECK(wait_flags(F_PING), "server received \"ping\"");
  QT_CHECK(wait_flags(F_PONG), "client received \"pong\"");
  struct aws_task tc;
  aws_task_init(&tc, sockets_close, NULL, "close");
  aws_event_loop_schedule_task_now(loop, &tc);
  QT_CHECK(wait_flags(F_CLOSED), "sockets closed on the loop");
  aws_socket_clean_up(&client);
  if (accepted) { aws_socket_clean_up(accepted); aws_mem_release(al, accepted); }
  aws_socket_clean_up(&listener);

  struct aws_byte_cursor data = aws_byte_cursor_from_c_str("0123456789abcdef");
  struct aws_input_stream *is = aws_input_stream_new_from_cursor(al, &data);
  int64_t len = 0;
  aws_input_stream_get_length(is, &len);
  char rb[8]; struct aws_byte_buf b = aws_byte_buf_from_empty_array(rb, 5);
  aws_input_stream_read(is, &b);
  QT_CHECK(len == 16 && b.len == 5 && memcmp(rb, "01234", 5) == 0, "input stream length and first read");
  aws_input_stream_seek(is, -3, AWS_SSB_END);
  b = aws_byte_buf_from_empty_array(rb, 8);
  aws_input_stream_read(is, &b);
  struct aws_stream_status ss;
  aws_input_stream_get_status(is, &ss);
  QT_CHECK(b.len == 3 && memcmp(rb, "def", 3) == 0 && ss.is_end_of_stream, "seek from end, then EOF");
  aws_input_stream_release(is);

  struct aws_array_list objs;
  QT_CHECK(aws_pem_objects_init_from_file_contents(&objs, al, aws_byte_cursor_from_c_str(pem_text)) == AWS_OP_SUCCESS &&
           aws_array_list_length(&objs) == 2, "two PEM objects parsed around junk");
  struct aws_pem_object *o0 = NULL, *o1 = NULL;
  aws_array_list_get_at_ptr(&objs, (void **)&o0, 0);
  aws_array_list_get_at_ptr(&objs, (void **)&o1, 1);
  QT_CHECK(o0 && o0->type == AWS_PEM_TYPE_X509 && o0->data.len == 16 && o0->data.buffer[15] == 15, "certificate decoded");
  QT_CHECK(o1 && o1->type == AWS_PEM_TYPE_PRIVATE_PKCS8 && o1->data.len == 5 && memcmp(o1->data.buffer, "Hello", 5) == 0,
           "private key decoded");
  aws_pem_objects_clean_up(&objs);

  aws_event_loop_group_release(elg);
  aws_io_library_clean_up();
  QT_END();
}
