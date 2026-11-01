/* APR (Apache Portable Runtime): pools and strings, tables and hashes, time
 * conversion (a known epoch), files and directories in a temporary
 * directory, threads with a mutex and atomics, a cross-process lock (APR's
 * default, fcntl) and anonymous shared memory between a forked child and
 * this process, base64, random bytes, and TCP on 127.0.0.1: a non-blocking
 * listener, accept with a timeout, and the check that APR's idea of an
 * accepted socket's O_NONBLOCK flag (configure's cross-compile answer
 * "inherited from the listener") matches what the socket really has. */
#include "common.h"
#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>
#include <apr_hash.h>
#include <apr_time.h>
#include <apr_file_io.h>
#include <apr_file_info.h>
#include <apr_thread_proc.h>
#include <apr_thread_mutex.h>
#include <apr_proc_mutex.h>
#include <apr_atomic.h>
#include <apr_shm.h>
#include <apr_network_io.h>
#include <apr_poll.h>
#include <apr_portable.h>
#include <apr_encode.h>
#include <apr_version.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

static apr_thread_mutex_t *g_mx;
static long g_count;
static apr_uint32_t g_atom;

static void *APR_THREAD_FUNC worker(apr_thread_t *t, void *arg) {
  int i;
  (void)arg;
  for (i = 0; i < 20000; i++) {
    apr_thread_mutex_lock(g_mx); g_count++; apr_thread_mutex_unlock(g_mx);
    apr_atomic_inc32(&g_atom);
  }
  apr_thread_exit(t, APR_SUCCESS);
  return NULL;
}

int main(void) {
  apr_pool_t *p;
  char buf[256], tmpl[512];
  apr_status_t rv;
  QT_BEGIN("apr");
  QT_CHECK(apr_initialize() == APR_SUCCESS, "apr_initialize()");
  QT_CHECK(apr_pool_create(&p, NULL) == APR_SUCCESS, "apr_pool_create()");
  printf("  info APR %s\n", apr_version_string());

  /* strings */
  QT_CHECK(strcmp(apr_psprintf(p, "%s-%d-%" APR_INT64_T_FMT, "qnx", 8, (apr_int64_t)1 << 40), "qnx-8-1099511627776") == 0,
           "apr_psprintf with APR_INT64_T_FMT");
  {
    char *s = apr_pstrdup(p, "a,b,,c"), *last, *tok; int n = 0;
    for (tok = apr_strtok(s, ",", &last); tok; tok = apr_strtok(NULL, ",", &last)) n++;
    QT_CHECK(n == 3, "apr_strtok splits a,b,,c into 3 tokens");
  }
  QT_CHECK(apr_strnatcmp("file10", "file9") > 0, "apr_strnatcmp: file10 > file9");
  apr_strfsize(1536 * 1024, buf);
  QT_CHECK(strcmp(buf, "1.5M") == 0, "apr_strfsize(1.5 MiB) = 1.5M");

  /* tables and hashes */
  {
    apr_table_t *t = apr_table_make(p, 4);
    apr_hash_t *h = apr_hash_make(p);
    apr_table_set(t, "Content-Type", "text/plain");
    apr_table_add(t, "X-A", "1"); apr_table_merge(t, "X-A", "2");
    QT_CHECK(strcmp(apr_table_get(t, "content-type"), "text/plain") == 0, "apr_table_get is case-insensitive");
    QT_CHECK(strcmp(apr_table_get(t, "X-A"), "1, 2") == 0, "apr_table_merge joins with ', '");
    apr_hash_set(h, "one", APR_HASH_KEY_STRING, "1");
    apr_hash_set(h, "two", APR_HASH_KEY_STRING, "2");
    QT_CHECK(apr_hash_count(h) == 2 && strcmp(apr_hash_get(h, "two", APR_HASH_KEY_STRING), "2") == 0, "apr_hash set/get/count");
  }

  /* time: 1234567890 s is 2009-02-13 23:31:30 UTC, a Friday */
  {
    apr_time_exp_t e; apr_time_t back;
    apr_time_exp_gmt(&e, apr_time_from_sec(1234567890));
    QT_CHECK(e.tm_year + 1900 == 2009 && e.tm_mon == 1 && e.tm_mday == 13 && e.tm_hour == 23 && e.tm_min == 31 &&
             e.tm_sec == 30 && e.tm_wday == 5, "apr_time_exp_gmt(1234567890) = Fri 2009-02-13 23:31:30");
    apr_time_exp_gmt_get(&back, &e);
    QT_CHECK(back == apr_time_from_sec(1234567890), "apr_time_exp_gmt_get round trip");
    apr_rfc822_date(buf, apr_time_from_sec(1234567890));
    QT_CHECK(strcmp(buf, "Fri, 13 Feb 2009 23:31:30 GMT") == 0, "apr_rfc822_date");
  }

  /* files and directories */
  {
    const char *tmp; char *dir, *path; apr_file_t *f; apr_finfo_t fi; apr_size_t n;
    apr_dir_t *d; int seen = 0;
    QT_CHECK(apr_temp_dir_get(&tmp, p) == APR_SUCCESS, "apr_temp_dir_get");
    dir = apr_psprintf(p, "%s/qnx-apr-%d", tmp, (int)getpid());
    QT_CHECK(apr_dir_make_recursive(apr_pstrcat(p, dir, "/sub", NULL), APR_FPROT_OS_DEFAULT, p) == APR_SUCCESS,
             "apr_dir_make_recursive");
    path = apr_pstrcat(p, dir, "/data.txt", NULL);
    rv = apr_file_open(&f, path, APR_FOPEN_CREATE | APR_FOPEN_WRITE | APR_FOPEN_TRUNCATE | APR_FOPEN_BUFFERED,
                       APR_FPROT_OS_DEFAULT, p);
    QT_CHECK(rv == APR_SUCCESS, "apr_file_open for writing");
    if (rv == APR_SUCCESS) {
      apr_file_printf(f, "line one\nline two\n"); apr_file_close(f);
    }
    QT_CHECK(apr_stat(&fi, path, APR_FINFO_SIZE | APR_FINFO_TYPE, p) == APR_SUCCESS && fi.size == 18 && fi.filetype == APR_REG,
             "apr_stat: regular file of 18 bytes");
    if (apr_file_open(&f, path, APR_FOPEN_READ, 0, p) == APR_SUCCESS) {
      apr_file_gets(buf, sizeof buf, f); apr_file_gets(buf, sizeof buf, f);
      QT_CHECK(strcmp(buf, "line two\n") == 0, "apr_file_gets reads the second line");
      n = 4; apr_file_seek(f, APR_SET, &(apr_off_t){5});
      QT_CHECK(apr_file_read(f, buf, &n) == APR_SUCCESS && n == 4 && memcmp(buf, "one\n", 4) == 0, "apr_file_seek + apr_file_read");
      apr_file_close(f);
    }
    snprintf(tmpl, sizeof tmpl, "%s/tmpXXXXXX", dir);
    QT_CHECK(apr_file_mktemp(&f, tmpl, 0, p) == APR_SUCCESS && strstr(tmpl, "XXXXXX") == NULL, "apr_file_mktemp");
    if (apr_dir_open(&d, dir, p) == APR_SUCCESS) {
      while (apr_dir_read(&fi, APR_FINFO_NAME | APR_FINFO_TYPE, d) == APR_SUCCESS) {
        if (!strcmp(fi.name, "sub") && fi.filetype == APR_DIR) seen |= 1;
        if (!strcmp(fi.name, "data.txt") && fi.filetype == APR_REG) seen |= 2;
      }
      apr_dir_close(d);
    }
    QT_CHECK(seen == 3, "apr_dir_read lists sub/ and data.txt with their types");
    { char cmd[600]; snprintf(cmd, sizeof cmd, "rm -rf '%s'", dir); system(cmd); }
  }

  /* threads, mutex, atomics */
  {
    apr_thread_t *th[4]; apr_status_t trv; int i, ok = 1;
    apr_thread_mutex_create(&g_mx, APR_THREAD_MUTEX_DEFAULT, p);
    for (i = 0; i < 4; i++) ok &= apr_thread_create(&th[i], NULL, worker, NULL, p) == APR_SUCCESS;
    for (i = 0; i < 4; i++) apr_thread_join(&trv, th[i]);
    QT_CHECK(ok && g_count == 80000, "4 threads x 20000 mutex-protected increments = 80000");
    QT_CHECK(apr_atomic_read32(&g_atom) == 80000, "apr_atomic_inc32 from 4 threads = 80000");
    QT_CHECK(apr_atomic_cas32(&g_atom, 7, 80000) == 80000 && apr_atomic_read32(&g_atom) == 7, "apr_atomic_cas32");
  }

  /* cross-process lock and anonymous shared memory with a child */
  {
    apr_shm_t *shm; apr_proc_mutex_t *pm; volatile int *cell; apr_proc_t child;
    char *lockf = apr_psprintf(p, "/tmp/qnx-apr-lock-%d", (int)getpid());
    QT_CHECK(apr_shm_create(&shm, sizeof(int) * 2, NULL, p) == APR_SUCCESS, "apr_shm_create (anonymous)");
    QT_CHECK(apr_proc_mutex_create(&pm, lockf, APR_LOCK_DEFAULT, p) == APR_SUCCESS, "apr_proc_mutex_create (default)");
    printf("  info default process lock: %s\n", apr_proc_mutex_name(pm));
    cell = apr_shm_baseaddr_get(shm); cell[0] = 0;
    rv = apr_proc_fork(&child, p);
    if (rv == APR_INCHILD) {
      int i;
      apr_proc_mutex_child_init(&pm, lockf, p);
      for (i = 0; i < 1000; i++) { apr_proc_mutex_lock(pm); cell[0]++; apr_proc_mutex_unlock(pm); }
      cell[1] = 4242;
      _exit(0);
    } else if (rv == APR_INPARENT) {
      int i, code = -1; apr_exit_why_e why;
      for (i = 0; i < 1000; i++) { apr_proc_mutex_lock(pm); cell[0]++; apr_proc_mutex_unlock(pm); }
      apr_proc_wait(&child, &code, &why, APR_WAIT);
      QT_CHECK(why == APR_PROC_EXIT && code == 0, "forked child exited 0");
      QT_CHECK(cell[1] == 4242, "child's write to shared memory is seen by the parent");
      QT_CHECK(cell[0] == 2000, "parent + child 1000 locked increments each = 2000");
    } else QT_CHECK(0, "apr_proc_fork");
    apr_proc_mutex_destroy(pm); apr_shm_destroy(shm); unlink(lockf);
  }

  /* base64, random */
  {
    apr_size_t len; const char *e = apr_pencode_base64(p, "QNX 8.0", APR_ENCODE_STRING, APR_ENCODE_NONE, &len);
    unsigned char r1[16], r2[16];
    QT_CHECK(e && strcmp(e, "UU5YIDguMA==") == 0, "apr_pencode_base64(\"QNX 8.0\") = UU5YIDguMA==");
    QT_CHECK(apr_generate_random_bytes(r1, 16) == APR_SUCCESS && apr_generate_random_bytes(r2, 16) == APR_SUCCESS &&
             memcmp(r1, r2, 16) != 0, "apr_generate_random_bytes gives different bytes twice");
  }

  /* TCP on the loopback */
  {
    apr_sockaddr_t *sa; apr_socket_t *ls, *cs, *as = NULL; apr_port_t port = 0; apr_size_t n;
    apr_pollset_t *ps; const apr_pollfd_t *out; apr_int32_t nout = 0;
    apr_os_sock_t afd; int nb, flags;
    QT_CHECK(apr_sockaddr_info_get(&sa, "127.0.0.1", APR_INET, 0, 0, p) == APR_SUCCESS, "apr_sockaddr_info_get(127.0.0.1)");
    apr_socket_create(&ls, APR_INET, SOCK_STREAM, APR_PROTO_TCP, p);
    apr_socket_opt_set(ls, APR_SO_REUSEADDR, 1);
    QT_CHECK(apr_socket_bind(ls, sa) == APR_SUCCESS && apr_socket_listen(ls, 4) == APR_SUCCESS, "bind + listen on port 0");
    apr_socket_addr_get(&sa, APR_LOCAL, ls); port = sa->port;
    QT_CHECK(port > 0, "the listener got a port");
    apr_socket_timeout_set(ls, 0);           /* non-blocking listener */
    rv = apr_socket_accept(&as, ls, p);
    QT_CHECK(APR_STATUS_IS_EAGAIN(rv), "accept on a non-blocking listener with nothing pending: EAGAIN");
    apr_sockaddr_info_get(&sa, "127.0.0.1", APR_INET, port, 0, p);
    apr_socket_create(&cs, APR_INET, SOCK_STREAM, APR_PROTO_TCP, p);
    QT_CHECK(apr_socket_connect(cs, sa) == APR_SUCCESS, "connect to the listener");
    apr_pollset_create(&ps, 4, p, 0);
    printf("  info pollset method: %s\n", apr_pollset_method_name(ps));
    {
      apr_pollfd_t pfd = { p, APR_POLL_SOCKET, APR_POLLIN, 0, { NULL }, NULL };
      pfd.desc.s = ls; apr_pollset_add(ps, &pfd);
    }
    QT_CHECK(apr_pollset_poll(ps, apr_time_from_sec(5), &nout, &out) == APR_SUCCESS && nout == 1, "pollset: the listener is readable");
    rv = apr_socket_accept(&as, ls, p);
    QT_CHECK(rv == APR_SUCCESS, "accept the connection");
    if (rv == APR_SUCCESS) {
      apr_os_sock_get(&afd, as);
      flags = fcntl(afd, F_GETFL);
      apr_socket_opt_get(as, APR_SO_NONBLOCK, &nb);
      printf("  info accepted socket: O_NONBLOCK %s, APR thinks %s\n", (flags & O_NONBLOCK) ? "set" : "clear", nb ? "set" : "clear");
      QT_CHECK(((flags & O_NONBLOCK) != 0) == (nb != 0), "APR's O_NONBLOCK bookkeeping of an accepted socket matches the socket");
      apr_socket_timeout_set(as, apr_time_from_sec(5));
      n = 5; apr_socket_send(cs, "hello", &n);
      n = sizeof buf; memset(buf, 0, sizeof buf);
      QT_CHECK(apr_socket_recv(as, buf, &n) == APR_SUCCESS && n == 5 && !memcmp(buf, "hello", 5), "send/recv 'hello' over the loopback");
      apr_socket_timeout_set(as, apr_time_from_msec(200));
      n = sizeof buf;
      QT_CHECK(APR_STATUS_IS_TIMEUP(apr_socket_recv(as, buf, &n)), "recv with a 200 ms timeout and no data: TIMEUP");
      apr_socket_close(as);
    }
    apr_socket_close(cs); apr_socket_close(ls);
  }
  apr_pool_destroy(p);
  apr_terminate();
  QT_END();
}
