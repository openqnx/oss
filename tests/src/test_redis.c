/* redis: redis-server started on 127.0.0.1 (a private directory in /tmp,
 * no persistence configured), driven by a RESP client written here and by
 * redis-cli. Checks: PING, SET/GET with binary data, INCR, lists, hashes,
 * sorted sets, EXPIRE/TTL, MULTI/EXEC, a Lua script (EVAL), PUBLISH reaching
 * a SUBSCRIBEd second connection, INFO naming the version, redis-cli setting
 * and reading a key, a short redis-benchmark run, SAVE writing dump.rdb that
 * redis-check-rdb accepts, BGSAVE (a forked child), and a restart that loads
 * the keys back from the RDB file. The server is stopped on every exit path
 * (atexit, a watchdog alarm, and fatal signals). */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static pid_t server = -1;
static char dir[64];
static const char *BINDIR[] = { "/usr/local/bin/", "", NULL };

static void stop_server(void) {
  if (server > 0) { kill(server, SIGKILL); waitpid(server, NULL, 0); server = -1; }
}
static void on_signal(int sig) {
  if (server > 0) kill(server, SIGKILL);
  if (sig == SIGALRM) { static const char m[] = "FAIL redis (watchdog: test took too long)\n"; (void)!write(1, m, sizeof m - 1); }
  _exit(2);
}
/* Run a redis program: argv[0] is the program name, looked up in BINDIR. */
static pid_t spawn(char *const argv[], const char *out) {
  pid_t p = fork();
  if (p == 0) {
    if (out) { FILE *f = freopen(out, "w", stdout); (void)f; dup2(1, 2); }
    char path[256];
    for (int i = 0; BINDIR[i]; i++) {
      snprintf(path, sizeof path, "%s%s", BINDIR[i], argv[0]);
      if (BINDIR[i][0]) execv(path, argv); else execvp(argv[0], argv);
    }
    _exit(127);
  }
  return p;
}
static int run_wait(char *const argv[], const char *out) {
  int st = -1; pid_t p = spawn(argv, out);
  if (p < 0) return -1;
  waitpid(p, &st, 0);
  return WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st);
}
static char *slurp(const char *path) {
  static char b[16384]; FILE *f = fopen(path, "r"); size_t n = 0;
  if (f) { n = fread(b, 1, sizeof b - 1, f); fclose(f); }
  b[n] = 0; return b;
}

/* ---- a minimal RESP client ---------------------------------------------- */
static int dial(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  if (connect(fd, (struct sockaddr *)&a, sizeof a) != 0) { close(fd); return -1; }
  struct timeval tv = { 15, 0 }; setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  return fd;
}
static char rbuf[65536]; static int rlen, rpos;
static int getbyte(int fd) {
  if (rpos == rlen) { rlen = (int)read(fd, rbuf, sizeof rbuf); rpos = 0; if (rlen <= 0) { rlen = 0; return -1; } }
  return (unsigned char)rbuf[rpos++];
}
static int getline_(int fd, char *out, int cap) {
  int n = 0, c;
  while ((c = getbyte(fd)) >= 0) {
    if (c == '\r') { getbyte(fd); break; }
    if (n < cap - 1) out[n++] = (char)c;
  }
  out[n] = 0; return c < 0 ? -1 : n;
}
/* Reads one reply and renders it flat into out: "+OK", ":5", "$hello",
   "$(nil)", "*[a,b]", "-ERR ...". Returns 0, or -1 on a broken connection. */
static int read_reply(int fd, char *out, int cap) {
  char line[1024];
  if (getline_(fd, line, sizeof line) < 0) return -1;
  switch (line[0]) {
  case '+': case '-': case ':': snprintf(out, cap, "%s", line); return 0;
  case '$': {
    long n = atol(line + 1);
    if (n < 0) { snprintf(out, cap, "$(nil)"); return 0; }
    int k = 0; out[k++] = '$';
    for (long i = 0; i < n; i++) { int c = getbyte(fd); if (c < 0) return -1; if (k < cap - 1) out[k++] = (char)c; }
    out[k] = 0; getbyte(fd); getbyte(fd); return 0;
  }
  case '*': {
    long n = atol(line + 1); int k = snprintf(out, cap, "*[");
    for (long i = 0; i < n; i++) {
      char el[4096];
      if (read_reply(fd, el, sizeof el) < 0) return -1;
      k += snprintf(out + k, cap > k ? cap - k : 0, "%s%s", i ? "," : "", el + (el[0] == '$' || el[0] == ':' || el[0] == '+' ? 1 : 0));
    }
    snprintf(out + k, cap > k ? cap - k : 0, "]"); return 0;
  }
  }
  snprintf(out, cap, "?%s", line); return 0;
}
/* Sends a command given as argc strings (with explicit lengths). */
static int sendv(int fd, int argc, const char **argv, const size_t *lens) {
  char hdr[64]; int n = snprintf(hdr, sizeof hdr, "*%d\r\n", argc);
  if (write(fd, hdr, n) != n) return -1;
  for (int i = 0; i < argc; i++) {
    size_t l = lens ? lens[i] : strlen(argv[i]);
    n = snprintf(hdr, sizeof hdr, "$%zu\r\n", l);
    if (write(fd, hdr, n) != n || write(fd, argv[i], l) != (ssize_t)l || write(fd, "\r\n", 2) != 2) return -1;
  }
  return 0;
}
static char reply[16384];
#define CMD(fd, ...) cmd_(fd, (const char *[]){ __VA_ARGS__, NULL })
static const char *cmd_(int fd, const char **argv) {
  int argc = 0; while (argv[argc]) argc++;
  if (sendv(fd, argc, argv, NULL) < 0 || read_reply(fd, reply, sizeof reply) < 0) snprintf(reply, sizeof reply, "(connection lost)");
  return reply;
}
static int is(const char *got, const char *want) {
  if (strcmp(got, want) == 0) return 1;
  printf("  info got %s, wanted %s\n", got, want);
  return 0;
}

static pid_t start(int port) {
  char p[16]; snprintf(p, sizeof p, "%d", port);
  char log[128]; snprintf(log, sizeof log, "%s/redis.log", dir);
  char *argv[] = { "redis-server", "--port", p, "--bind", "127.0.0.1", "--dir", dir,
                   "--save", "", "--appendonly", "no", "--daemonize", "no", "--protected-mode", "no",
                   "--logfile", log, "--maxclients", "64", NULL };
  return spawn(argv, NULL);
}
static int wait_up(int port) {
  for (int i = 0; i < 150; i++) {
    int st, fd = dial(port);
    if (fd >= 0) { rlen = rpos = 0; const char *r = CMD(fd, "PING"); int ok = !strcmp(r, "+PONG"); close(fd); if (ok) return 1; }
    if (waitpid(server, &st, WNOHANG) == server) { printf("  info redis-server exited early, status 0x%x\n", st); server = -1; return 0; }
    usleep(100000);
  }
  return 0;
}

int main(void) {
  QT_BEGIN("redis");
  snprintf(dir, sizeof dir, "/tmp/qnxport-redis-%d", (int)getpid());
  mkdir(dir, 0700);
  atexit(stop_server);
  signal(SIGALRM, on_signal); signal(SIGINT, on_signal); signal(SIGTERM, on_signal);
  signal(SIGSEGV, on_signal); signal(SIGABRT, on_signal); signal(SIGPIPE, SIG_IGN);
  alarm(150);
  int port = 20000 + getpid() % 20000;
  char ps[16]; snprintf(ps, sizeof ps, "%d", port);
  char path[160], out[160];

  server = start(port);
  int up = wait_up(port);
  QT_CHECK(up, "redis-server answers PING on 127.0.0.1");
  if (up) {
    int fd = dial(port); rlen = rpos = 0;
    QT_CHECK(is(CMD(fd, "SET", "greeting", "hello qnx"), "+OK") && is(CMD(fd, "GET", "greeting"), "$hello qnx"), "SET/GET");
    {
      const char *argv[] = { "SET", "bin", "a\0b\r\nc" }; size_t lens[] = { 3, 3, 6 };
      sendv(fd, 3, argv, lens); read_reply(fd, reply, sizeof reply);
      CMD(fd, "STRLEN", "bin");
      QT_CHECK(is(reply, ":6"), "binary-safe value (NUL, CR LF inside)");
    }
    QT_CHECK(is(CMD(fd, "GET", "nokey"), "$(nil)"), "GET of a missing key: nil");
    CMD(fd, "SET", "n", "40"); CMD(fd, "INCR", "n");
    QT_CHECK(is(CMD(fd, "INCRBY", "n", "1"), ":42"), "INCR/INCRBY: 42");
    QT_CHECK(CMD(fd, "INCR", "greeting")[0] == '-', "INCR on a non-integer: error reply");
    CMD(fd, "RPUSH", "list", "a", "b", "c"); CMD(fd, "LPUSH", "list", "z");
    QT_CHECK(is(CMD(fd, "LRANGE", "list", "0", "-1"), "*[z,a,b,c]"), "lists: RPUSH/LPUSH/LRANGE");
    CMD(fd, "HSET", "h", "f1", "v1", "f2", "v2");
    QT_CHECK(is(CMD(fd, "HGET", "h", "f2"), "$v2") && is(CMD(fd, "HLEN", "h"), ":2"), "hashes: HSET/HGET/HLEN");
    CMD(fd, "ZADD", "z", "3", "three", "1", "one", "2", "two");
    QT_CHECK(is(CMD(fd, "ZRANGE", "z", "0", "-1", "WITHSCORES"), "*[one,1,two,2,three,3]"), "sorted set ordered by score");
    CMD(fd, "SET", "tmp", "x"); CMD(fd, "EXPIRE", "tmp", "100");
    CMD(fd, "TTL", "tmp"); int ttl = atoi(reply + 1);
    QT_CHECK(ttl > 90 && ttl <= 100, "EXPIRE/TTL");
    CMD(fd, "PEXPIRE", "tmp", "50"); usleep(300000);
    QT_CHECK(is(CMD(fd, "EXISTS", "tmp"), ":0"), "a key with a 50 ms expiry is gone after 300 ms");
    CMD(fd, "MULTI"); CMD(fd, "INCR", "tx"); CMD(fd, "INCR", "tx");
    QT_CHECK(is(CMD(fd, "EXEC"), "*[1,2]"), "MULTI/EXEC");
    QT_CHECK(is(CMD(fd, "EVAL", "return redis.call('INCRBY', KEYS[1], ARGV[1]) * 2", "1", "n", "8"), ":100"), "EVAL Lua script: (42+8)*2 = 100");
    QT_CHECK(is(CMD(fd, "EVAL", "return cjson.encode({1,2,3})", "0"), "$[1,2,3]"), "Lua cjson library");

    /* pub/sub on a second connection */
    {
      /* replies are read whole, so the shared read buffer is empty here */
      int sub = dial(port);
      rlen = rpos = 0;
      CMD(sub, "SUBSCRIBE", "news");
      int subscribed = strstr(reply, "subscribe,news") != NULL;
      CMD(fd, "PUBLISH", "news", "breaking");
      QT_CHECK(subscribed && is(reply, ":1"), "PUBLISH reaches one subscriber");
      read_reply(sub, reply, sizeof reply);
      QT_CHECK(is(reply, "*[message,news,breaking]"), "the subscriber receives the message");
      close(sub);
    }
    CMD(fd, "INFO", "server");
    char *v = strstr(reply, "redis_version:");
    QT_CHECK(v && !strncmp(v + 14, "8.10.3", 6), "INFO server: redis_version 8.10.3");
    char *os = strstr(reply, "os:"); if (os) { char *e = strchr(os, '\r'); if (e) *e = 0; printf("  info %s\n", os); }

    /* redis-cli */
    snprintf(out, sizeof out, "%s/cli.out", dir);
    char *cli1[] = { "redis-cli", "-h", "127.0.0.1", "-p", ps, "SET", "clikey", "from redis-cli", NULL };
    int rc = run_wait(cli1, out);
    QT_CHECK(rc == 0 && !strncmp(slurp(out), "OK", 2), "redis-cli SET prints OK");
    QT_CHECK(is(CMD(fd, "GET", "clikey"), "$from redis-cli"), "the key redis-cli set is there");
    char *cli2[] = { "redis-cli", "-h", "127.0.0.1", "-p", ps, "--no-raw", "LRANGE", "list", "0", "1", NULL };
    rc = run_wait(cli2, out);
    QT_CHECK(rc == 0 && strstr(slurp(out), "1) \"z\"") && strstr(slurp(out), "2) \"a\""), "redis-cli formats a list reply");
    char *bench[] = { "redis-benchmark", "-h", "127.0.0.1", "-p", ps, "-n", "2000", "-c", "4", "-t", "set,get", "-q", NULL };
    rc = run_wait(bench, out);
    char *bo = slurp(out); char *nl = strchr(bo, '\n'); if (nl) *nl = 0;
    char *cr = strrchr(bo, '\r'); if (cr) bo = cr + 1;   /* drop the progress lines */
    printf("  info redis-benchmark: %s\n", bo);
    QT_CHECK(rc == 0 && strstr(bo, "requests per second"), "redis-benchmark SET/GET run (2000 requests)");

    /* persistence */
    CMD(fd, "DBSIZE"); int keys = atoi(reply + 1);
    QT_CHECK(is(CMD(fd, "SAVE"), "+OK"), "SAVE");
    snprintf(path, sizeof path, "%s/dump.rdb", dir);
    struct stat st;
    QT_CHECK(stat(path, &st) == 0 && st.st_size > 0, "dump.rdb written");
    char *chk[] = { "redis-check-rdb", path, NULL };
    rc = run_wait(chk, out);
    QT_CHECK(rc == 0 && strstr(slurp(out), "RDB looks OK"), "redis-check-rdb: RDB looks OK");
    CMD(fd, "LASTSAVE"); long last = atol(reply + 1);
    sleep(1);
    CMD(fd, "SET", "after-save", "1");
    const char *bg = CMD(fd, "BGSAVE");
    QT_CHECK(!strcmp(bg, "+Background saving started"), "BGSAVE started (forked child)");
    if (bg[0] == '-') printf("  info BGSAVE: %s\n", bg);
    long now = last;
    for (int i = 0; i < 100 && now == last; i++) { usleep(100000); CMD(fd, "LASTSAVE"); now = atol(reply + 1); }
    QT_CHECK(now > last, "BGSAVE completed (LASTSAVE advanced)");
    CMD(fd, "INFO", "persistence");
    QT_CHECK(strstr(reply, "rdb_last_bgsave_status:ok") != NULL, "rdb_last_bgsave_status:ok");
    keys++;
    CMD(fd, "SHUTDOWN", "NOSAVE");
    close(fd);
    int ws = 0, exited = 0;
    for (int i = 0; i < 100; i++) { if (waitpid(server, &ws, WNOHANG) == server) { exited = 1; break; } usleep(100000); }
    QT_CHECK(exited, "SHUTDOWN NOSAVE stops the server");
    if (exited) server = -1; else stop_server();

    /* restart: the keys come back from dump.rdb */
    server = start(port + 1);
    up = wait_up(port + 1);
    QT_CHECK(up, "restarted redis-server answers");
    if (up) {
      fd = dial(port + 1); rlen = rpos = 0;
      CMD(fd, "DBSIZE");
      QT_CHECK(atoi(reply + 1) == keys, "restart loads every key from dump.rdb");
      QT_CHECK(is(CMD(fd, "GET", "greeting"), "$hello qnx") && is(CMD(fd, "HGET", "h", "f1"), "$v1") &&
               is(CMD(fd, "GET", "after-save"), "$1"), "values survive the restart (SAVE and BGSAVE data)");
      CMD(fd, "SHUTDOWN", "NOSAVE"); close(fd);
      for (int i = 0; i < 100 && server > 0; i++) { if (waitpid(server, NULL, WNOHANG) == server) server = -1; else usleep(100000); }
    }
  }
  stop_server();
  if (qt_fail_count) {
    snprintf(path, sizeof path, "%s/redis.log", dir);
    FILE *f = fopen(path, "r"); char line[512]; int n = 0;
    while (f && fgets(line, sizeof line, f)) if (n++ < 60) printf("  info log: %s", line);
    if (f) fclose(f);
  }
  char cmd[128]; snprintf(cmd, sizeof cmd, "rm -rf %s", dir); (void)!system(cmd);
  QT_END();
}
