// imath: vectors, matrices, quaternions and the half type, with known answers:
// dot and cross products, a 90-degree rotation matrix applied to a point, a
// matrix inverse, quaternion rotation, half-float rounding and bit patterns,
// and the library's (not inline) functions -- succf/predf, Rand32.
#include "common.h"
#include <Imath/ImathVec.h>
#include <Imath/ImathMatrix.h>
#include <Imath/ImathQuat.h>
#include <Imath/ImathRandom.h>
#include <Imath/ImathFun.h>
#include <Imath/half.h>
#include <cmath>

using namespace IMATH_NAMESPACE;

static bool near(double a, double b, double e = 1e-6) { return std::fabs(a - b) < e; }

int main() {
  QT_BEGIN("imath");
  V3f a(1, 2, 3), b(4, 5, 6);
  QT_CHECK(a.dot(b) == 32.0f, "(1,2,3).(4,5,6) = 32");
  V3f c = a.cross(b);
  QT_CHECK(c == V3f(-3, 6, -3), "(1,2,3)x(4,5,6) = (-3,6,-3)");
  QT_CHECK(near(V3f(3, 4, 0).length(), 5.0), "|(3,4,0)| = 5");

  M44f r; r.setAxisAngle(V3f(0, 0, 1), float(M_PI / 2));
  V3f p = V3f(1, 0, 0) * r;
  QT_CHECK(near(p.x, 0, 1e-6) && near(p.y, 1, 1e-6) && near(p.z, 0, 1e-6), "rotating (1,0,0) 90 deg about z gives (0,1,0)");
  M44d m(2, 0, 0, 0,  0, 4, 0, 0,  0, 0, 8, 0,  1, 2, 3, 1);
  M44d mi = m.inverse();
  M44d id = m * mi;
  QT_CHECK(id.equalWithAbsError(M44d(), 1e-12), "M * M^-1 = identity");
  QT_CHECK(near(m.determinant(), 64.0), "det(diag 2,4,8 with translation) = 64");

  Quatf q; q.setAxisAngle(V3f(0, 1, 0), float(M_PI));
  V3f pq = q.rotateVector(V3f(1, 0, 0));
  QT_CHECK(near(pq.x, -1, 1e-6) && near(pq.z, 0, 1e-5), "quaternion: 180 deg about y maps x to -x");

  half h(1.0f);
  QT_CHECK(h.bits() == 0x3c00, "half(1.0) bits are 0x3c00");
  half h3(0.333333f);
  QT_CHECK(near(float(h3), 0.333251953125, 1e-12), "half(1/3) rounds to 0.333251953125");
  QT_CHECK(half(65504.0f).bits() == 0x7bff, "half max 65504 is 0x7bff");
  QT_CHECK(half(1e6f).isInfinity(), "1e6 overflows half to infinity");

  QT_CHECK(succf(1.0f) > 1.0f && predf(1.0f) < 1.0f && succf(predf(1.0f)) == 1.0f, "succf/predf step one ulp");
  Rand32 r1(42), r2(42);
  bool same = true; for (int i = 0; i < 100; i++) if (r1.nextf() != r2.nextf()) same = false;
  QT_CHECK(same, "Rand32 with one seed gives one sequence");
  QT_CHECK(clamp(7, 0, 5) == 5 && lerp(0.0, 10.0, 0.25) == 2.5, "clamp and lerp");
  QT_END();
}
