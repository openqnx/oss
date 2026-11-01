/* libassuan: an Assuan server (forked child, on one end of a socketpair) and
 * a client in the parent. Checks the protocol round trip with known answers:
 * the server's greeting, a registered ECHO command returning its argument as
 * a data line (percent-escaped by the server, decoded by the client),
 * a status line, INQUIRE answered by the client, built-in NOP, an unknown
 * command's error code, and a command handler's error coming back as ERR;
 * plus assuan's own percent-escape helpers. */
#include "common.h"
#include <assuan.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

static gpg_error_t cmd_echo(assuan_context_t ctx, char *line) {
  /* Echo the argument back unchanged in a data line. */
  gpg_error_t err = assuan_send_data(ctx, line, strlen(line));
  if (!err) err = assuan_write_status(ctx, "ECHOED", "1");
  return err;
}
static gpg_error_t cmd_ask(assuan_context_t ctx, char *line) {
  (void)line;
  unsigned char *buf = NULL; size_t len = 0;
  gpg_error_t err = assuan_inquire(ctx, "NUMBER", &buf, &len, 100);
  if (!err) {
    /* reply with the inquired number times two */
    char out[32]; snprintf(out, sizeof out, "%ld", 2 * strtol((char *)buf, NULL, 10));
    assuan_free(ctx, buf);
    err = assuan_send_data(ctx, out, strlen(out));
  }
  return err;
}
static gpg_error_t cmd_fail(assuan_context_t ctx, char *line) {
  (void)line;
  return gpg_error(GPG_ERR_NOT_SUPPORTED);
}

static int server(int fd) {
  assuan_context_t ctx;
  if (assuan_new(&ctx)) return 1;
  if (assuan_init_socket_server(ctx, assuan_fd_from_posix_fd(fd), ASSUAN_SOCKET_SERVER_ACCEPTED)) return 2;
  assuan_set_hello_line(ctx, "qnx test server ready");
  if (assuan_register_command(ctx, "ECHO", cmd_echo, NULL) ||
      assuan_register_command(ctx, "ASK", cmd_ask, NULL) ||
      assuan_register_command(ctx, "FAIL", cmd_fail, NULL)) return 3;
  if (assuan_accept(ctx)) return 4;
  gpg_error_t err = assuan_process(ctx);
  assuan_release(ctx);
  return err ? 5 : 0;
}

struct rx { char data[256]; size_t n; char status[128]; };
static gpg_error_t data_cb(void *opaque, const void *buf, size_t len) {
  struct rx *r = opaque;
  if (r->n + len < sizeof r->data) { memcpy(r->data + r->n, buf, len); r->n += len; r->data[r->n] = 0; }
  return 0;
}
static gpg_error_t inq_cb(void *opaque, const char *line) {
  assuan_context_t ctx = opaque;
  if (strncmp(line, "NUMBER", 6) != 0) return gpg_error(GPG_ERR_ASS_UNKNOWN_INQUIRE);
  return assuan_send_data(ctx, "21", 2);
}
static gpg_error_t status_cb(void *opaque, const char *line) {
  struct rx *r = opaque;
  snprintf(r->status, sizeof r->status, "%s", line);
  return 0;
}

int main(void) {
  QT_BEGIN("libassuan");
  /* as gpg-agent does: a peer that has gone away is EPIPE, not a signal */
  signal(SIGPIPE, SIG_IGN);
  const char *v = assuan_check_version(NULL);
  printf("  info assuan_check_version: %s\n", v ? v : "(null)");
  QT_CHECK(v && strcmp(v, "3.0.2") == 0, "library version 3.0.2");

  QT_CHECK(ASSUAN_LINELENGTH == 1002, "ASSUAN_LINELENGTH is 1002");

  int sv[2];
  QT_CHECK(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0, "socketpair");
  pid_t pid = fork();
  if (pid == 0) { close(sv[0]); _exit(server(sv[1])); }
  close(sv[1]);
  QT_CHECK(pid > 0, "server forked");

  assuan_context_t ctx;
  QT_CHECK(assuan_new(&ctx) == 0, "assuan_new (client)");
  gpg_error_t err = assuan_socket_connect_fd(ctx, assuan_fd_from_posix_fd(sv[0]), 0);
  QT_CHECK(err == 0, "client connected; server greeting accepted");
  if (err) { printf("  info %s\n", gpg_strerror(err)); kill(pid, SIGKILL); waitpid(pid, NULL, 0); QT_END(); }

  struct rx r; memset(&r, 0, sizeof r);
  err = assuan_transact(ctx, "ECHO hello QNX", data_cb, &r, NULL, NULL, status_cb, &r);
  printf("  info ECHO -> data \"%s\", status \"%s\"\n", r.data, r.status);
  QT_CHECK(err == 0 && strcmp(r.data, "hello QNX") == 0, "ECHO returns \"hello QNX\" as data");
  QT_CHECK(strcmp(r.status, "ECHOED 1") == 0, "status line \"ECHOED 1\"");

  memset(&r, 0, sizeof r);
  err = assuan_transact(ctx, "ECHO 100% sure", data_cb, &r, NULL, NULL, NULL, NULL);
  QT_CHECK(err == 0 && strcmp(r.data, "100% sure") == 0, "a '%' survives the data line's percent-escaping");

  memset(&r, 0, sizeof r);
  err = assuan_transact(ctx, "ASK", data_cb, &r, inq_cb, ctx, NULL, NULL);
  QT_CHECK(err == 0 && strcmp(r.data, "42") == 0, "INQUIRE NUMBER answered with 21; server replies 42");

  QT_CHECK(assuan_transact(ctx, "NOP", NULL, NULL, NULL, NULL, NULL, NULL) == 0, "built-in NOP is OK");
  err = assuan_transact(ctx, "NOSUCHCMD", NULL, NULL, NULL, NULL, NULL, NULL);
  QT_CHECK(gpg_err_code(err) == GPG_ERR_ASS_UNKNOWN_CMD, "unknown command -> GPG_ERR_ASS_UNKNOWN_CMD");
  err = assuan_transact(ctx, "FAIL", NULL, NULL, NULL, NULL, NULL, NULL);
  QT_CHECK(gpg_err_code(err) == GPG_ERR_NOT_SUPPORTED, "handler error comes back as ERR (Not supported)");

  QT_CHECK(assuan_transact(ctx, "BYE", NULL, NULL, NULL, NULL, NULL, NULL) == 0, "BYE acknowledged");
  assuan_release(ctx);
  int st = -1;
  waitpid(pid, &st, 0);
  printf("  info server exit status %d\n", WIFEXITED(st) ? WEXITSTATUS(st) : -1);
  QT_CHECK(WIFEXITED(st) && WEXITSTATUS(st) == 0, "server ended cleanly after BYE");
  QT_END();
}
