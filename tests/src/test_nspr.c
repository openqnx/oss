/* NSPR: formatted output, time conversion, base64, hash tables, threads
 * with a lock and a condition variable, and a TCP echo over loopback --
 * all with known answers. */
#include "common.h"
#include <nspr.h>
#include <plbase64.h>
#include <plhash.h>
#include <plstr.h>
#include <prprf.h>
#include <stdlib.h>

/* ---- threads: a counter under a lock, a producer/consumer queue ---- */
#define NTHREADS 4
#define NINCR 10000
static PRLock *lock;
static PRCondVar *cv;
static int counter, queue[64], qlen, qdone;

static void incr(void *arg) {
  (void)arg;
  for (int i = 0; i < NINCR; i++) { PR_Lock(lock); counter++; PR_Unlock(lock); }
}

static void producer(void *arg) {
  (void)arg;
  for (int i = 1; i <= 50; i++) {
    PR_Lock(lock);
    queue[qlen++] = i;
    PR_NotifyCondVar(cv);
    PR_Unlock(lock);
  }
  PR_Lock(lock); qdone = 1; PR_NotifyCondVar(cv); PR_Unlock(lock);
}

/* ---- TCP echo server: accepts one client and echoes what it reads ---- */
static PRFileDesc *listener;
static void echo_server(void *arg) {
  (void)arg;
  PRNetAddr peer;
  PRFileDesc *c = PR_Accept(listener, &peer, PR_SecondsToInterval(20));
  if (!c) return;
  char buf[256];
  PRInt32 n;
  while ((n = PR_Recv(c, buf, sizeof buf, 0, PR_SecondsToInterval(20))) > 0)
    PR_Send(c, buf, n, 0, PR_SecondsToInterval(20));
  PR_Close(c);
}

static PRIntn cmp_str(const void *a, const void *b) { return strcmp(a, b) == 0; }
static PRIntn cmp_val(const void *a, const void *b) { return a == b; }

int main(void) {
  QT_BEGIN("nspr");
  PR_Init(PR_USER_THREAD, PR_PRIORITY_NORMAL, 0);
  QT_CHECK(PR_Initialized(), "PR_Init");
  QT_CHECK(PR_VersionCheck(PR_VERSION), "PR_VersionCheck(" PR_VERSION ")");
  printf("  info NSPR %s\n", PR_GetVersion());

  /* PR_snprintf: NSPR's own printf engine, incl. 64-bit and padding */
  char s[128];
  PR_snprintf(s, sizeof s, "%d|%5s|%-4s|%lld|%x|%.3f", -42, "ab", "cd",
              (PRInt64)1234567890123LL, 255u, 3.14159);
  QT_CHECK(strcmp(s, "-42|   ab|cd  |1234567890123|ff|3.142") == 0, "PR_snprintf formats");
  char *m = PR_smprintf("%s-%u", "nspr", 439u);
  QT_CHECK(m && strcmp(m, "nspr-439") == 0, "PR_smprintf allocates");
  PR_smprintf_free(m);

  /* Time: 1700000000 s after the epoch is 2023-11-14 22:13:20 UTC, a Tuesday */
  PRExplodedTime et;
  PRTime t = (PRTime)1700000000 * PR_USEC_PER_SEC;
  PR_ExplodeTime(t, PR_GMTParameters, &et);
  QT_CHECK(et.tm_year == 2023 && et.tm_month == 10 && et.tm_mday == 14 &&
           et.tm_hour == 22 && et.tm_min == 13 && et.tm_sec == 20 && et.tm_wday == 2,
           "PR_ExplodeTime(1700000000) = Tue 2023-11-14 22:13:20 UTC");
  QT_CHECK(PR_ImplodeTime(&et) == t, "PR_ImplodeTime round trip");
  PRTime parsed;
  QT_CHECK(PR_ParseTimeString("Tue, 14 Nov 2023 22:13:20 GMT", PR_FALSE, &parsed) == PR_SUCCESS
           && parsed == t, "PR_ParseTimeString (RFC 1123 date)");
  PR_FormatTimeUSEnglish(s, sizeof s, "%Y-%m-%d %H:%M:%S %a", &et);
  QT_CHECK(strcmp(s, "2023-11-14 22:13:20 Tue") == 0, "PR_FormatTimeUSEnglish");
  QT_CHECK(PR_Now() > t, "PR_Now is after 2023");

  /* plc / plds: base64, string helpers, hash table */
  char *b = PL_Base64Encode("Hello, NSPR!", 12, NULL);
  QT_CHECK(b && strcmp(b, "SGVsbG8sIE5TUFIh") == 0, "PL_Base64Encode");
  char *d = PL_Base64Decode("SGVsbG8sIE5TUFIh", 16, NULL);
  QT_CHECK(d && memcmp(d, "Hello, NSPR!", 12) == 0, "PL_Base64Decode");
  PR_Free(b); PR_Free(d);
  QT_CHECK(PL_strcasecmp("NsPr", "nspr") == 0 && PL_strlen("abcdef") == 6, "PL_strcasecmp / PL_strlen");
  PLHashTable *ht = PL_NewHashTable(8, PL_HashString, cmp_str, cmp_val, NULL, NULL);
  static const char *keys[] = { "alpha", "beta", "gamma", "delta" };
  for (int i = 0; i < 4; i++) PL_HashTableAdd(ht, keys[i], (void *)(long)(i + 1));
  QT_CHECK(PL_HashTableLookup(ht, "gamma") == (void *)3L && PL_HashTableLookup(ht, "omega") == NULL,
           "PLHashTable lookup");
  PL_HashTableRemove(ht, "beta");
  QT_CHECK(PL_HashTableLookup(ht, "beta") == NULL, "PLHashTable remove");
  PL_HashTableDestroy(ht);

  /* Threads */
  lock = PR_NewLock(); cv = PR_NewCondVar(lock);
  PRThread *th[NTHREADS];
  for (int i = 0; i < NTHREADS; i++)
    th[i] = PR_CreateThread(PR_USER_THREAD, incr, NULL, PR_PRIORITY_NORMAL,
                            PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
  int joined = 0;
  for (int i = 0; i < NTHREADS; i++) joined += th[i] && PR_JoinThread(th[i]) == PR_SUCCESS;
  QT_CHECK(joined == NTHREADS && counter == NTHREADS * NINCR, "4 threads x 10000 locked increments = 40000");

  PRThread *p = PR_CreateThread(PR_USER_THREAD, producer, NULL, PR_PRIORITY_NORMAL,
                                PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
  int sum = 0, got = 0;
  PR_Lock(lock);
  for (;;) {
    while (qlen == 0 && !qdone) PR_WaitCondVar(cv, PR_SecondsToInterval(10));
    while (qlen > 0) { sum += queue[--qlen]; got++; }
    if (qdone) break;
  }
  PR_Unlock(lock);
  PR_JoinThread(p);
  QT_CHECK(got == 50 && sum == 1275, "condvar producer/consumer: 50 items, sum 1275");

  /* TCP echo on 127.0.0.1 */
  listener = PR_NewTCPSocket();
  PRNetAddr addr;
  PR_InitializeNetAddr(PR_IpAddrLoopback, 0, &addr);
  int ok = listener && PR_Bind(listener, &addr) == PR_SUCCESS && PR_Listen(listener, 4) == PR_SUCCESS
           && PR_GetSockName(listener, &addr) == PR_SUCCESS;
  QT_CHECK(ok, "TCP listen on 127.0.0.1 (ephemeral port)");
  if (ok) {
    printf("  info listening on port %u\n", PR_ntohs(addr.inet.port));
    PRThread *srv = PR_CreateThread(PR_USER_THREAD, echo_server, NULL, PR_PRIORITY_NORMAL,
                                    PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 0);
    PRFileDesc *c = PR_NewTCPSocket();
    QT_CHECK(c && PR_Connect(c, &addr, PR_SecondsToInterval(10)) == PR_SUCCESS, "PR_Connect");
    const char msg[] = "hello over NSPR sockets";
    char buf[64] = {0};
    PRInt32 n = PR_Send(c, msg, sizeof msg, 0, PR_SecondsToInterval(10)), r = 0;
    while (r < (PRInt32)sizeof msg) {
      PRInt32 k = PR_Recv(c, buf + r, sizeof buf - r, 0, PR_SecondsToInterval(10));
      if (k <= 0) break;
      r += k;
    }
    QT_CHECK(n == (PRInt32)sizeof msg && r == (PRInt32)sizeof msg && memcmp(buf, msg, sizeof msg) == 0,
             "TCP echo round trip");
    /* PR_Poll: nothing more to read until the server closes after our shutdown */
    PRPollDesc pd = { c, PR_POLL_READ, 0 };
    QT_CHECK(PR_Poll(&pd, 1, PR_MillisecondsToInterval(100)) == 0, "PR_Poll times out with no data");
    PR_Shutdown(c, PR_SHUTDOWN_SEND);
    QT_CHECK(PR_Recv(c, buf, sizeof buf, 0, PR_SecondsToInterval(10)) == 0, "EOF after the server closes");
    PR_Close(c);
    PR_JoinThread(srv);
  }
  if (listener) PR_Close(listener);

  /* Address parsing / formatting */
  PRNetAddr na;
  QT_CHECK(PR_StringToNetAddr("192.168.10.20", &na) == PR_SUCCESS && PR_NetAddrToString(&na, s, sizeof s) == PR_SUCCESS
           && strcmp(s, "192.168.10.20") == 0 && na.inet.ip == PR_htonl(0xC0A80A14), "PR_StringToNetAddr / PR_NetAddrToString");

  PR_DestroyCondVar(cv); PR_DestroyLock(lock);
  QT_CHECK(PR_Cleanup() == PR_SUCCESS, "PR_Cleanup");
  QT_END();
}
