/* Qhull (libqhull_r, the reentrant library): the convex hull of the unit
 * square's corners plus interior points has exactly the 4 corners as
 * vertices, 4 facets and area 1; the unit cube's 8 corners plus its centre
 * give 8 hull vertices, volume 1 and area 6; the Delaunay triangulation of
 * the square plus its centre has 4 triangles. The qhull program computes
 * the same 2-d hull from its standard input (stdout checked). */
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libqhull_r/qhull_ra.h>

static int hull(int dim, int n, coordT *pts, const char *opts, int *nv, int *nf, double *area, double *vol) {
  qhT qh_qh, *qh = &qh_qh;
  qh_zero(qh, stderr);
  char cmd[64]; snprintf(cmd, sizeof cmd, "qhull %s", opts);
  int exitcode = qh_new_qhull(qh, dim, n, pts, False, cmd, NULL, stderr);
  if (!exitcode) {
    *nv = qh->num_vertices; *nf = qh->num_facets;
    if (area) { qh_getarea(qh, qh->facet_list); *area = qh->totarea; *vol = qh->totvol; }
    if (opts[0] == 'd') {   /* Delaunay: count the lower-hull (non-upper) simplicial facets */
      facetT *facet; int c = 0;
      FORALLfacets { if (!facet->upperdelaunay) c++; }
      *nf = c;
    }
  }
  qh_freeqhull(qh, !qh_ALL);
  int curlong, totlong; qh_memfreeshort(qh, &curlong, &totlong);
  return exitcode;
}

int main(void) {
  QT_BEGIN("qhull");
  printf("  info %s\n", qh_version);
  coordT sq[] = { 0, 0, 1, 0, 1, 1, 0, 1, 0.5, 0.5, 0.25, 0.75, 0.6, 0.1 };
  int nv = 0, nf = 0; double area = 0, vol = 0;
  QT_CHECK(hull(2, 7, sq, "", &nv, &nf, &area, &vol) == 0, "2-d hull computed");
  QT_CHECK(nv == 4 && nf == 4, "square + 3 interior points: 4 vertices, 4 edges");
  QT_CHECK(fabs(vol - 1.0) < 1e-12 && fabs(area - 4.0) < 1e-12, "area 1, perimeter 4");
  coordT cube[27]; int k = 0;
  for (int i = 0; i < 8; i++) { cube[k++] = i & 1; cube[k++] = (i >> 1) & 1; cube[k++] = (i >> 2) & 1; }
  cube[k++] = 0.5; cube[k++] = 0.5; cube[k++] = 0.5;
  QT_CHECK(hull(3, 9, cube, "Qt", &nv, &nf, &area, &vol) == 0, "3-d hull computed");
  printf("  info cube: %d vertices, %d facets (triangulated)\n", nv, nf);
  QT_CHECK(nv == 8 && nf == 12, "cube + centre: 8 vertices, 12 triangles");
  QT_CHECK(fabs(vol - 1.0) < 1e-12 && fabs(area - 6.0) < 1e-12, "volume 1, area 6");
  coordT del[] = { 0, 0, 1, 0, 1, 1, 0, 1, 0.5, 0.5 };
  QT_CHECK(hull(2, 5, del, "d Qbb Qc Qz Q12", &nv, &nf, NULL, NULL) == 0, "Delaunay triangulation computed");
  QT_CHECK(nf == 4, "square + centre: 4 Delaunay triangles");
  char pathv[1024]; snprintf(pathv, sizeof pathv, "/usr/local/bin:%s", getenv("PATH") ? getenv("PATH") : "/bin");
  setenv("PATH", pathv, 1);
  FILE *p = popen("printf '2\\n5\\n0 0\\n2 0\\n2 2\\n0 2\\n1 1\\n' | qhull Fx 2>&1", "r");
  char line[128]; int count = -1, got = 0, ids[8];
  if (p) {
    if (fgets(line, sizeof line, p)) count = atoi(line);
    while (got < 8 && fgets(line, sizeof line, p)) ids[got++] = atoi(line);
    pclose(p);
  }
  int sum = 0; for (int i = 0; i < got; i++) sum += ids[i];
  QT_CHECK(count == 4 && got == 4 && sum == 0 + 1 + 2 + 3, "qhull Fx: hull points 0..3 of the square, not the centre");
  QT_END();
}
