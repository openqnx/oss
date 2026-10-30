#include "common.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <mujoco/mujoco.h>
/* MuJoCo: a model compiled from MJCF -- a 0.1 m ball 1 m above a plane --
 * and simulated. Free fall: at t = 0.3 s, z = 1 - g t^2 / 2 = 0.5586 m
 * (before it touches anything); after 3 s it rests on the plane, z = 0.1. */
static const char *xml =
  "<mujoco><option timestep='0.002' gravity='0 0 -9.81'/>"
  "<worldbody><geom type='plane' size='5 5 0.1'/>"
  "<body pos='0 0 1'><freejoint/><geom type='sphere' size='0.1' mass='1'/></body>"
  "</worldbody></mujoco>";
int main(void) {
  QT_BEGIN("mujoco");
  printf("  info MuJoCo %s\n", mj_versionString());
  char path[] = "/tmp/mjball-XXXXXX"; int fd = mkstemp(path);
  FILE *f = fdopen(fd, "w"); fputs(xml, f); fclose(f);
  char err[1000] = "";
  mjModel *m = mj_loadXML(path, NULL, err, sizeof err);
  unlink(path);
  QT_CHECK(m != NULL, "mj_loadXML (MJCF compiled)");
  if (!m) { printf("  info %s\n", err); QT_END(); }
  QT_CHECK(m->nbody == 2 && m->ngeom == 2 && m->nq == 7, "2 bodies, 2 geoms, free joint (nq 7)");
  mjData *d = mj_makeData(m);
  while (d->time < 0.3 - 1e-9) mj_step(m, d);
  double z = d->qpos[2], want = 1.0 - 0.5 * 9.81 * 0.3 * 0.3;
  printf("  info t=%.3f z=%.4f (free fall: %.4f)\n", d->time, z, want);
  QT_CHECK(fabs(z - want) < 0.005, "free fall at 0.3 s matches 1 - g t^2 / 2");
  while (d->time < 3.0) mj_step(m, d);
  printf("  info t=%.3f z=%.4f vz=%.5f\n", d->time, d->qpos[2], d->qvel[2]);
  QT_CHECK(fabs(d->qpos[2] - 0.1) < 0.005 && fabs(d->qvel[2]) < 0.01, "at rest on the plane (z = radius)");
  mj_deleteData(d); mj_deleteModel(m);
  QT_END();
}
