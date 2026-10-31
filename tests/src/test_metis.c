/* METIS: partition a graph whose best cut is known -- two 4-cliques joined
 * by a single edge -- with the C API (recursive bisection and k-way: both
 * must cut exactly that one edge and keep each clique whole), compute a
 * fill-reducing ordering (a valid permutation), then do the same through the
 * gpmetis / graphchk / ndmetis programs on a METIS graph file. */
#include "common.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <metis.h>

/* Vertices 0-3 and 4-7 are cliques; 3-4 is the bridge. CSR, 0-based. */
#define NV 8
static idx_t xadj[NV + 1], adjncy[64];
static int adj(int a, int b) {
  int ca = a < 4, cb = b < 4;
  if (a == b) return 0;
  if (ca == cb) return 1;
  return (a == 3 && b == 4) || (a == 4 && b == 3);
}
static void build(void) {
  int k = 0;
  for (int v = 0; v < NV; v++) {
    xadj[v] = k;
    for (int u = 0; u < NV; u++) if (adj(v, u)) adjncy[k++] = u;
  }
  xadj[NV] = k;
}
static int cliques_whole(const idx_t *part) {
  for (int v = 1; v < 4; v++) if (part[v] != part[0]) return 0;
  for (int v = 5; v < 8; v++) if (part[v] != part[4]) return 0;
  return part[0] != part[4];
}
static int run(const char *cmd, char *out, size_t n) {
  out[0] = 0;
  FILE *p = popen(cmd, "r");
  if (!p) return -1;
  size_t len = fread(out, 1, n - 1, p);
  out[len] = 0;
  int st = pclose(p);
  return WIFEXITED(st) ? WEXITSTATUS(st) : -1;
}

int main(void) {
  QT_BEGIN("metis");
  printf("  info METIS %d.%d.%d, idx_t %d-bit\n", METIS_VER_MAJOR, METIS_VER_MINOR,
         METIS_VER_SUBMINOR, IDXTYPEWIDTH);
  build();
  QT_CHECK(xadj[NV] == 2 * (6 + 6 + 1), "graph built: 13 edges");

  idx_t nv = NV, ncon = 1, nparts = 2, objval = -1, part[NV];
  idx_t options[METIS_NOPTIONS];
  METIS_SetDefaultOptions(options);
  options[METIS_OPTION_SEED] = 1;
  int rc = METIS_PartGraphRecursive(&nv, &ncon, xadj, adjncy, NULL, NULL, NULL, &nparts,
                                    NULL, NULL, options, &objval, part);
  printf("  info recursive: rc=%d edgecut=%d parts=", rc, (int)objval);
  for (int v = 0; v < NV; v++) printf("%d", (int)part[v]);
  printf("\n");
  QT_CHECK(rc == METIS_OK, "METIS_PartGraphRecursive returns METIS_OK");
  QT_CHECK(objval == 1, "bisection cuts exactly the one bridge edge");
  QT_CHECK(cliques_whole(part), "each clique stays in one part");

  objval = -1;
  for (int v = 0; v < NV; v++) part[v] = -1;
  rc = METIS_PartGraphKway(&nv, &ncon, xadj, adjncy, NULL, NULL, NULL, &nparts,
                           NULL, NULL, options, &objval, part);
  printf("  info k-way: rc=%d edgecut=%d\n", rc, (int)objval);
  QT_CHECK(rc == METIS_OK && objval == 1 && cliques_whole(part),
           "METIS_PartGraphKway finds the same 1-edge cut");

  idx_t perm[NV], iperm[NV], seen[NV] = {0};
  rc = METIS_NodeND(&nv, xadj, adjncy, NULL, NULL, perm, iperm);
  int okperm = rc == METIS_OK;
  for (int v = 0; v < NV && okperm; v++) {
    if (perm[v] < 0 || perm[v] >= NV || seen[perm[v]]++) okperm = 0;
    else if (iperm[perm[v]] != v) okperm = 0;
  }
  QT_CHECK(okperm, "METIS_NodeND gives a permutation and its inverse");

  /* The programs, on the same graph in METIS file format (1-based). */
  char dir[] = "/tmp/qnxport-metis.XXXXXX", path[256], cmd[512], out[4096];
  QT_CHECK(mkdtemp(dir) != NULL, "scratch directory");
  snprintf(path, sizeof path, "%s/g.graph", dir);
  FILE *f = fopen(path, "w");
  if (f) {
    fprintf(f, "%d %d\n", NV, 13);
    for (int v = 0; v < NV; v++) {
      for (idx_t k = xadj[v]; k < xadj[v + 1]; k++) fprintf(f, "%s%d", k == xadj[v] ? "" : " ", (int)adjncy[k] + 1);
      fprintf(f, "\n");
    }
    fclose(f);
  }
  snprintf(cmd, sizeof cmd, "graphchk %s 2>&1", path);
  rc = run(cmd, out, sizeof out);
  if (rc != 0 || !strstr(out, "correct")) printf("  info graphchk rc=%d: %s\n", rc, out);
  QT_CHECK(rc == 0 && strstr(out, "correct") != NULL, "graphchk accepts the graph file");
  /* If gpmetis crashes, these say how far it gets: argument parsing and the
     usage text only, then the partitioner without writing the file. */
  snprintf(cmd, sizeof cmd, "cd %s && gpmetis 2>&1", dir);
  rc = run(cmd, out, sizeof out);
  printf("  info gpmetis (no arguments) -> rc %d%s\n", rc, strstr(out, "Missing parameters") ? ", usage printed" : "");
  snprintf(cmd, sizeof cmd, "cd %s && gpmetis -nooutput g.graph 2 2>&1", dir);
  rc = run(cmd, out, sizeof out);
  printf("  info gpmetis -nooutput -> rc %d%s\n", rc, strstr(out, "Edgecut: 1,") ? ", Edgecut 1" : "");
  snprintf(cmd, sizeof cmd, "cd %s && gpmetis -seed=1 g.graph 2 2>&1", dir);
  rc = run(cmd, out, sizeof out);
  const char *ec = strstr(out, "Edgecut:");
  if (rc != 0 || !ec) printf("  info gpmetis rc=%d: %s\n", rc, out);
  QT_CHECK(rc == 0 && ec && atoi(ec + 8) == 1, "gpmetis reports Edgecut: 1");
  snprintf(path, sizeof path, "%s/g.graph.part.2", dir);
  idx_t fpart[NV];
  int n = 0, x;
  f = fopen(path, "r");
  if (f) { while (n < NV && fscanf(f, "%d", &x) == 1) fpart[n++] = x; fclose(f); }
  QT_CHECK(n == NV && cliques_whole(fpart), "gpmetis wrote g.graph.part.2 splitting the cliques");
  snprintf(cmd, sizeof cmd, "cd %s && ndmetis g.graph 2>&1", dir);
  rc = run(cmd, out, sizeof out);
  if (rc != 0) printf("  info ndmetis rc=%d: %s\n", rc, out);
  snprintf(path, sizeof path, "%s/g.graph.iperm", dir);
  QT_CHECK(rc == 0 && access(path, R_OK) == 0, "ndmetis writes g.graph.iperm");
  snprintf(cmd, sizeof cmd, "rm -rf %s", dir);
  if (system(cmd) != 0) printf("  info could not remove %s\n", dir);
  QT_END();
}
