/* GKlib (METIS's support library): sorting and key-value sorting, the BLAS-
 * like vector helpers, string pruning/replacement (incl. its regex-based
 * replace), the tokenizer, the max-priority queue, the seeded RNG's
 * reproducibility, file/dir tests, and a CSR matrix read from a file,
 * transposed and summed. */
#include "common.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <GKlib.h>

int main(void) {
  QT_BEGIN("gklib");
  int a[] = {5, -3, 9, 0, 7, 7, -8};
  gk_isorti(7, a);
  QT_CHECK(a[0] == -8 && a[1] == -3 && a[6] == 9 && a[4] == 7, "gk_isorti ascending");
  gk_isortd(7, a);
  QT_CHECK(a[0] == 9 && a[6] == -8, "gk_isortd descending");
  gk_ikv_t kv[4] = {{30, 0}, {10, 1}, {20, 2}, {5, 3}};
  gk_ikvsorti(4, kv);
  QT_CHECK(kv[0].key == 5 && kv[0].val == 3 && kv[3].key == 30 && kv[3].val == 0, "key-value sort keeps values with keys");

  double x[4] = {3, 4, 0, 0}, y[4] = {1, 2, 3, 4};
  QT_CHECK(gk_dnorm2(4, x, 1) == 5.0, "norm2(3,4,0,0) = 5");
  QT_CHECK(gk_ddot(4, x, 1, y, 1) == 11.0 && gk_dsum(4, y, 1) == 10.0, "dot and sum");
  QT_CHECK(gk_dargmax(4, y, 1) == 3 && gk_dmax(4, y, 1) == 4.0, "argmax / max");
  gk_daxpy(4, 2.0, y, 1, x, 1);
  QT_CHECK(x[0] == 5 && x[3] == 8, "axpy");
  int seq[5]; gk_iincset(5, 10, seq);
  QT_CHECK(seq[0] == 10 && seq[4] == 14, "incset");

  char s1[] = "  hello world\t\n";
  QT_CHECK(strcmp(gk_strhprune(gk_strtprune(s1, " \t\n"), " "), "hello world") == 0, "head/tail prune");
  char s2[] = "a-b_c-d";
  QT_CHECK(strcmp(gk_strchr_replace(s2, "-_", "+."), "a+b.c+d") == 0, "character replacement");
  char s3[] = "port qnx to qnx", *out = NULL;
  int rc = gk_strstr_replace(s3, "qnx", "QNX", "g", &out);
  QT_CHECK(rc >= 1 && out && strcmp(out, "port QNX to QNX") == 0, "regex global replace");
  gk_free((void **)&out, LTERM);
  char *dup = gk_strdup("Neutrino");
  QT_CHECK(dup && gk_strcasecmp(dup, "NEUTRINO") == 1, "strdup + case-insensitive equal");
  gk_free((void **)&dup, LTERM);

  gk_Tokens_t tok;
  char line[] = "alpha,beta;;gamma";
  gk_strtokenize(line, ",;", &tok);
  QT_CHECK(tok.ntoks == 3 && strcmp(tok.list[2], "gamma") == 0, "tokenizer collapses repeated delimiters");
  gk_freetokenslist(&tok);

  gk_ipq_t *pq = gk_ipqCreate(10);
  gk_ipqInsert(pq, 1, 40); gk_ipqInsert(pq, 2, 90); gk_ipqInsert(pq, 3, 10); gk_ipqInsert(pq, 4, 70);
  gk_ipqUpdate(pq, 3, 100);
  QT_CHECK(gk_ipqLength(pq) == 4 && gk_ipqSeeTopKey(pq) == 100, "priority queue top after update");
  gk_idx_t o1 = gk_ipqGetTop(pq), o2 = gk_ipqGetTop(pq), o3 = gk_ipqGetTop(pq);
  QT_CHECK(o1 == 3 && o2 == 2 && o3 == 4 && gk_ipqCheckHeap(pq), "max-heap pops 3, 2, 4");
  gk_ipqDestroy(pq);

  gk_randinit(4242); uint64_t r1 = gk_randint64(), r2 = gk_randint64();
  gk_randinit(4242); uint64_t r3 = gk_randint64();
  QT_CHECK(r1 == r3 && r1 != r2, "seeded RNG is reproducible");

  /* (gk_mkpath/gk_rmpath shell out to mkdir/rm through system(); not used) */
  char dir[] = "/tmp/qnxport-gklib", nodir[] = "/tmp/qnxport-gklib-none";
  mkdir(dir, 0755);
  QT_CHECK(gk_dexists(dir) && !gk_dexists(nodir) && !gk_fexists(dir), "dexists / fexists");
  char *bn = gk_getbasename("/tmp/qnxport-gklib/matrix.csr"), *ext = gk_getextname("/tmp/qnxport-gklib/matrix.csr");
  QT_CHECK(bn && strcmp(bn, "matrix") == 0 && ext && strcmp(ext, "csr") == 0, "basename / extension");
  gk_free((void **)&bn, &ext, LTERM);

  char fn[] = "/tmp/qnxport-gklib/matrix.csr";
  FILE *f = fopen(fn, "w");
  if (f) { fputs("0 1.0 2 2.0\n1 3.0\n1 4.0\n0 0.5 1 0.5 2 1.0\n", f); fclose(f); }
  gk_csr_t *m = gk_fexists(fn) ? gk_csr_Read(fn, GK_CSR_FMT_CSR, 1, 0) : NULL;
  QT_CHECK(m && m->nrows == 4 && m->ncols == 3 && m->rowptr[4] == 7, "CSR file read: 4 rows, 3 columns, 7 non-zeros");
  if (m) {
    gk_csr_ComputeSums(m, GK_CSR_ROW);
    QT_CHECK(m->rsums[0] == 3.0f && m->rsums[1] == 3.0f && m->rsums[2] == 4.0f && m->rsums[3] == 2.0f, "row sums");
    gk_csr_t *t = gk_csr_Transpose(m);
    QT_CHECK(t && t->nrows == 3 && t->rowptr[3] == 7 && t->rowptr[1] - t->rowptr[0] == 2 && t->rowptr[2] - t->rowptr[1] == 3, "transpose: columns hold 2 and 3 entries");
    gk_csr_Free(&t);
    gk_csr_Free(&m);
  }
  unlink(fn);
  rmdir(dir);
  QT_END();
}
