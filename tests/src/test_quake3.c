/* ioquake3 dedicated server, headless, on loopback. No game data ships, so:
 *  - without data the server must refuse to start with its "pak0.pk3 is
 *    missing" message (and exit, not hang); so must the client engine
 *    (ioquake3, linked with SDL2) started in dedicated mode;
 *  - with a tiny pk3 the test writes itself (a default.cfg and a minimal
 *    valid BSP map: one plane, two leafs, a worldspawn and a spawn point)
 *    as a standalone game directory, next to a link to the installed native
 *    game module (baseq3/qagame.so), the server loads the map and runs the
 *    game module. The test then talks the Quake III network protocol to it
 *    over UDP: getstatus and getinfo answers name the map, host name and
 *    protocol 71; rcon with the right password changes sv_hostname (seen in
 *    the next getstatus); a wrong password is refused. The server is
 *    stopped with SIGTERM on every exit path and must shut the game down.
 * Q3_DIR overrides the install directory (default /usr/local/opt/quake3). */
#include "common.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#ifdef __QNX__
#define QNX_OS_TAG " qnx-"
#else
#define QNX_OS_TAG " linux-"   /* a build-machine run of the test */
#endif

static pid_t server = -1;
static char tmpdir[256];

static void stop_server(void) {
  if (server > 0) {
    kill(server, SIGTERM);
    for (int i = 0; i < 100; ++i) {           /* 10 s */
      if (waitpid(server, NULL, WNOHANG) == server) { server = -1; return; }
      usleep(100000);
    }
    kill(server, SIGKILL); waitpid(server, NULL, 0); server = -1;
  }
}
static void cleanup(void) {
  stop_server();
  if (tmpdir[0]) { char cmd[300]; snprintf(cmd, sizeof cmd, "rm -rf '%s'", tmpdir); if (system(cmd)) {} }
}
static void on_signal(int s) { (void)s; cleanup(); _exit(2); }

/* ---- a stored (uncompressed) zip ---- */
static uint32_t crc32_of(const unsigned char *p, size_t n) {
  uint32_t c = 0xffffffffu;
  for (size_t i = 0; i < n; ++i) { c ^= p[i]; for (int k = 0; k < 8; ++k) c = (c >> 1) ^ (0xedb88320u & -(c & 1)); }
  return ~c;
}
static void put16(FILE *f, unsigned v) { fputc(v & 255, f); fputc((v >> 8) & 255, f); }
static void put32(FILE *f, uint32_t v) { put16(f, v & 0xffff); put16(f, v >> 16); }
struct zent { const char *name; const unsigned char *data; uint32_t len, crc, off; };
static int write_zip(const char *path, struct zent *e, int n) {
  FILE *f = fopen(path, "wb");
  if (!f) return 0;
  for (int i = 0; i < n; ++i) {
    e[i].crc = crc32_of(e[i].data, e[i].len); e[i].off = (uint32_t)ftell(f);
    put32(f, 0x04034b50); put16(f, 10); put16(f, 0); put16(f, 0); put16(f, 0); put16(f, 0x21);
    put32(f, e[i].crc); put32(f, e[i].len); put32(f, e[i].len); put16(f, strlen(e[i].name)); put16(f, 0);
    fputs(e[i].name, f); fwrite(e[i].data, 1, e[i].len, f);
  }
  uint32_t cd = (uint32_t)ftell(f);
  for (int i = 0; i < n; ++i) {
    put32(f, 0x02014b50); put16(f, 20); put16(f, 10); put16(f, 0); put16(f, 0); put16(f, 0); put16(f, 0x21);
    put32(f, e[i].crc); put32(f, e[i].len); put32(f, e[i].len); put16(f, strlen(e[i].name));
    put16(f, 0); put16(f, 0); put16(f, 0); put16(f, 0); put32(f, 0); put32(f, e[i].off); fputs(e[i].name, f);
  }
  uint32_t cdlen = (uint32_t)ftell(f) - cd;
  put32(f, 0x06054b50); put16(f, 0); put16(f, 0); put16(f, n); put16(f, n); put32(f, cdlen); put32(f, cd); put16(f, 0);
  return fclose(f) == 0;
}

/* ---- a minimal Quake III BSP (IBSP version 46) ---- */
static unsigned char bsp[2048];
static size_t bsp_len;
static void le32(unsigned char *p, int32_t v) { uint32_t u = (uint32_t)v; p[0] = u; p[1] = u >> 8; p[2] = u >> 16; p[3] = u >> 24; }
static void lef(unsigned char *p, float v) { int32_t i; memcpy(&i, &v, 4); le32(p, i); }
static void make_bsp(void) {
  static const char ents[] = "{\n\"classname\" \"worldspawn\"\n\"message\" \"QNX test room\"\n}\n"
                             "{\n\"classname\" \"info_player_deathmatch\"\n\"origin\" \"0 0 64\"\n}\n";
  unsigned char lump[17][160]; int len[17] = {0};
  memset(lump, 0, sizeof lump); memset(bsp, 0, sizeof bsp);
  len[0] = sizeof ents;                                                      /* entities, NUL included */
  memcpy(lump[0], ents, sizeof ents);
  strcpy((char *)lump[1], "textures/common/caulk"); le32(lump[1] + 68, 1); len[1] = 72;  /* shader: CONTENTS_SOLID */
  lef(lump[2] + 8, 1.f); len[2] = 16;                                         /* plane z = 0, normal +z */
  { int32_t n[9] = {0, -1, -2, -512, -512, -512, 512, 512, 512};             /* node: front leaf 0, back leaf 1 */
    for (int i = 0; i < 9; ++i) le32(lump[3] + 4 * i, n[i]);
    len[3] = 36; }
  { int32_t l[24] = {0, 0, -512, -512, 0, 512, 512, 512, 0, 0, 0, 0,          /* leafs: empty above, solid below */
                     -1, -1, -512, -512, -512, 512, 512, 0, 0, 0, 0, 0};
    for (int i = 0; i < 24; ++i) le32(lump[4] + 4 * i, l[i]);
    len[4] = 96; }
  { float b[6] = {-512, -512, -512, 512, 512, 512};                           /* model 0: the world */
    for (int i = 0; i < 6; ++i) lef(lump[7] + 4 * i, b[i]);
    len[7] = 40; }
  memcpy(bsp, "IBSP", 4); le32(bsp + 4, 46);
  size_t off = 8 + 17 * 8;
  for (int i = 0; i < 17; ++i) {
    int l = (len[i] + 3) & ~3;
    le32(bsp + 8 + 8 * i, (int32_t)off); le32(bsp + 12 + 8 * i, len[i]);
    memcpy(bsp + off, lump[i], len[i]); off += l;
  }
  bsp_len = off;
}

/* ---- the server and the protocol ---- */
static pid_t spawn(const char *bin, char *const argv[], const char *log) {
  pid_t p = fork();
  if (p == 0) {
    int fd = open(log, O_WRONLY | O_CREAT | O_TRUNC, 0644), nul = open("/dev/null", O_RDONLY);
    if (fd >= 0) { dup2(fd, 1); dup2(fd, 2); }
    if (nul >= 0) dup2(nul, 0);
    execv(bin, argv);
    _exit(127);
  }
  return p;
}
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec / 1e9; }
/* Sends an out-of-band packet and waits for an answer starting with 'expect'. */
static int oob(int s, int port, const char *cmd, const char *expect, char *out, size_t outsz, double wait_s) {
  struct sockaddr_in a; memset(&a, 0, sizeof a);
  a.sin_family = AF_INET; a.sin_port = htons(port); a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  /* Not snprintf("\xff\xff\xff\xff%s"): QNX's printf family reads the
     format as UTF-8 and stops at the first invalid byte (EILSEQ, returning
     the count so far), so that sent a 1-byte packet. */
  char pkt[512]; size_t cl = strlen(cmd);
  if (cl > sizeof pkt - 4) cl = sizeof pkt - 4;
  memset(pkt, 0xff, 4); memcpy(pkt + 4, cmd, cl);
  int n = (int)(4 + cl);
  double end = now() + wait_s;
  while (now() < end) {
    if (server > 0 && waitpid(server, NULL, WNOHANG) == server) { server = -1; return 0; }
    sendto(s, pkt, n, 0, (struct sockaddr *)&a, sizeof a);
    double until = now() + 0.6;           /* the server rate-limits queries per address */
    while (now() < until) {
      ssize_t r = recv(s, out, outsz - 1, 0);
      if (r > 4) {
        out[r] = 0;
        if (strncmp(out + 4, expect, strlen(expect)) == 0) return 1;
      }
    }
  }
  return 0;
}
static int file_has(const char *path, const char *needle) {
  FILE *f = fopen(path, "r"); if (!f) return 0;
  static char buf[1 << 17]; size_t n = fread(buf, 1, sizeof buf - 1, f); fclose(f); buf[n] = 0;
  return strstr(buf, needle) != NULL;
}
static void show_log(const char *path) {
  FILE *f = fopen(path, "r"); char line[512]; int k = 0;
  if (!f) return;
  while (fgets(line, sizeof line, f) && k++ < 60) printf("  log  %s", line);
  fclose(f);
}

int main(void) {
  QT_BEGIN("quake3");
  const char *dir = getenv("Q3_DIR") ? getenv("Q3_DIR") : "/usr/local/opt/quake3";
  char ded[512], game[512];
  snprintf(ded, sizeof ded, "%s/ioq3ded", dir);
  snprintf(game, sizeof game, "%s/baseq3/qagame.so", dir);
  QT_CHECK(access(ded, X_OK) == 0 && access(game, R_OK) == 0, "ioq3ded and baseq3/qagame.so are installed");

  snprintf(tmpdir, sizeof tmpdir, "%s/qnxport-q3.XXXXXX", getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp");
  if (!mkdtemp(tmpdir)) { tmpdir[0] = 0; QT_CHECK(0, "temporary directory"); QT_END(); }
  atexit(cleanup);
  signal(SIGINT, on_signal); signal(SIGTERM, on_signal); signal(SIGHUP, on_signal);

  char p[700], p2[700], home[700], log[700], port_s[16];
  int port = 27960 + 100 + (int)(getpid() % 500);
  snprintf(port_s, sizeof port_s, "%d", port);
  snprintf(home, sizeof home, "%s/home", tmpdir); mkdir(home, 0755);
  snprintf(p, sizeof p, "%s/empty", tmpdir); mkdir(p, 0755);

  /* 1. no game data: a clear refusal, and the process ends */
  snprintf(log, sizeof log, "%s/nodata.log", tmpdir);
  {
    char *argv[] = {ded, "+set", "fs_basepath", p, "+set", "fs_homepath", home, "+set", "ttycon", "0",
                    "+set", "net_enabled", "0", "+set", "dedicated", "1", NULL};
    server = spawn(ded, argv, log);
    int st = -1; double end = now() + 60;
    while (now() < end && waitpid(server, &st, WNOHANG) != server) usleep(100000);
    int ended = now() < end; if (ended) server = -1; else stop_server();
    QT_CHECK(ended, "without game data the server exits by itself");
    QT_CHECK(file_has(log, "pak0.pk3"), "and says pak0.pk3 is missing");
    if (!file_has(log, "pak0.pk3")) show_log(log);
  }

  /* 1b. the client engine (SDL2) starts too: dedicated mode needs no
     display, and it refuses missing data the same way */
  {
    char cli[512]; snprintf(cli, sizeof cli, "%s/ioquake3", dir);
    snprintf(log, sizeof log, "%s/client.log", tmpdir);
    char *argv[] = {cli, "+set", "fs_basepath", p, "+set", "fs_homepath", home, "+set", "ttycon", "0",
                    "+set", "net_enabled", "0", "+set", "dedicated", "1", NULL};
    server = spawn(cli, argv, log);
    int st = -1; double end = now() + 60;
    while (now() < end && waitpid(server, &st, WNOHANG) != server) usleep(100000);
    int ended = now() < end; if (ended) server = -1; else stop_server();
    QT_CHECK(ended && file_has(log, "pak0.pk3") && file_has(log, "ioq3"),
             "the client engine (ioquake3) starts in dedicated mode and refuses missing data");
    if (!(ended && file_has(log, "pak0.pk3"))) show_log(log);
  }

  /* 2. a standalone game directory with a generated pk3 */
  char gdir[600]; snprintf(gdir, sizeof gdir, "%s/base/qnxtest", tmpdir);
  snprintf(p, sizeof p, "%s/base", tmpdir); mkdir(p, 0755); mkdir(gdir, 0755);
  make_bsp();
  static const char cfg[] = "set sv_hostname \"qnx-q3-test\"\nset g_motd \"hello from QNX\"\n";
  struct zent z[2] = {{"default.cfg", (const unsigned char *)cfg, sizeof cfg - 1, 0, 0},
                      {"maps/qnx.bsp", bsp, (uint32_t)bsp_len, 0, 0}};
  snprintf(p2, sizeof p2, "%s/pak0.pk3", gdir);
  QT_CHECK(write_zip(p2, z, 2), "wrote a pk3 with default.cfg and maps/qnx.bsp");
  snprintf(p2, sizeof p2, "%s/qagame.so", gdir);
  QT_CHECK(symlink(game, p2) == 0, "linked the native game module into the game directory");

  snprintf(log, sizeof log, "%s/server.log", tmpdir);
  char *argv[] = {ded, "+set", "fs_basepath", p, "+set", "fs_homepath", home, "+set", "com_basegame", "qnxtest",
                  "+set", "vm_game", "0", "+set", "bot_enable", "0", "+set", "ttycon", "0",
                  "+set", "net_ip", "127.0.0.1", "+set", "net_enabled", "1", "+set", "net_port", port_s,
                  "+set", "rconpassword", "qnxpw", "+set", "dedicated", "1", "+map", "qnx", NULL};
  server = spawn(ded, argv, log);
  int s = socket(AF_INET, SOCK_DGRAM, 0);
  struct timeval tv = {0, 200000}; setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
  static char r[8192];
  int got = oob(s, port, "getstatus", "statusResponse", r, sizeof r, 90);
  QT_CHECK(got, "getstatus is answered (the map is running)");
  if (got) {
    printf("  info %.200s\n", r + 4);
    QT_CHECK(strstr(r, "\\mapname\\qnx\\") != NULL, "status: mapname qnx");
    QT_CHECK(strstr(r, "\\sv_hostname\\qnx-q3-test\\") != NULL, "status: sv_hostname from the pk3's default.cfg");
    QT_CHECK(strstr(r, "\\com_protocol\\71\\") != NULL && strstr(r, QNX_OS_TAG) != NULL,
             "status: protocol 71, version string names QNX");
    QT_CHECK(strstr(r, "\\gamename\\baseq3\\") != NULL, "status: the game module answers (gamename baseq3)");
    got = oob(s, port, "getinfo qnxchallenge", "infoResponse", r, sizeof r, 10);
    QT_CHECK(got && strstr(r, "\\challenge\\qnxchallenge") && strstr(r, "\\clients\\0") && strstr(r, "\\mapname\\qnx"),
             "getinfo echoes the challenge, 0 clients, map qnx");
    got = oob(s, port, "rcon wrongpw status", "print", r, sizeof r, 10);
    QT_CHECK(got && strstr(r, "Bad rconpassword") != NULL, "rcon with a wrong password is refused");
    sleep(1);                                   /* rcon is rate-limited */
    got = oob(s, port, "rcon qnxpw set sv_hostname renamed-by-rcon", "print", r, sizeof r, 10);
    QT_CHECK(got, "rcon with the password is accepted");
    got = oob(s, port, "getstatus", "statusResponse", r, sizeof r, 10);
    QT_CHECK(got && strstr(r, "\\sv_hostname\\renamed-by-rcon\\") != NULL, "rcon changed sv_hostname");
  }
  close(s);
  int alive = server > 0 && waitpid(server, NULL, WNOHANG) == 0;
  QT_CHECK(alive, "server still running after the queries");
  stop_server();
  QT_CHECK(file_has(log, "Game Initialization"), "the native game module was initialised");
  QT_CHECK(file_has(log, "ShutdownGame"), "SIGTERM shuts the game down");
  if (qt_fail_count) show_log(log);
  QT_END();
}
