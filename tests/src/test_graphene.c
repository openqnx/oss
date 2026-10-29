/* graphene: the SIMD vector/matrix maths GTK4 renders with. Its types are
 * 16-byte aligned and it selects an SSE/NEON implementation at build time, so
 * a wrong alignment or a missing intrinsic shows up as wrong arithmetic. */
#include "common.h"
#include <graphene.h>
#include <math.h>
static int close_to(float a, float b) { return fabsf(a - b) < 1e-4f; }
int main(void) {
  QT_BEGIN("graphene");
  graphene_vec3_t a, b, c;
  graphene_vec3_init(&a, 1.f, 0.f, 0.f);
  graphene_vec3_init(&b, 0.f, 1.f, 0.f);
  graphene_vec3_cross(&a, &b, &c);
  QT_CHECK(close_to(graphene_vec3_get_z(&c), 1.f), "x cross y == z");
  QT_CHECK(close_to(graphene_vec3_dot(&a, &b), 0.f), "orthogonal vectors dot to 0");
  QT_CHECK(close_to(graphene_vec3_length(&a), 1.f), "unit vector length");

  graphene_matrix_t m, inv;
  graphene_matrix_init_identity(&m);
  graphene_matrix_translate(&m, &GRAPHENE_POINT3D_INIT(2.f, 3.f, 4.f));
  graphene_point3d_t p = GRAPHENE_POINT3D_INIT(1.f, 1.f, 1.f), out;
  graphene_matrix_transform_point3d(&m, &p, &out);
  QT_CHECK(close_to(out.x, 3.f) && close_to(out.y, 4.f) && close_to(out.z, 5.f),
           "translation applied to a point");
  QT_CHECK(graphene_matrix_inverse(&m, &inv), "matrix inverse");
  graphene_matrix_transform_point3d(&inv, &out, &p);
  QT_CHECK(close_to(p.x, 1.f) && close_to(p.y, 1.f) && close_to(p.z, 1.f),
           "inverse round trip returns the original point");

  graphene_rect_t r1 = GRAPHENE_RECT_INIT(0, 0, 10, 10);
  graphene_rect_t r2 = GRAPHENE_RECT_INIT(5, 5, 10, 10), is;
  QT_CHECK(graphene_rect_intersection(&r1, &r2, &is), "rect intersection");
  QT_CHECK(close_to(is.size.width, 5.f) && close_to(is.size.height, 5.f),
           "intersection is 5x5");
  QT_CHECK(graphene_rect_contains_point(&r1, &GRAPHENE_POINT_INIT(3, 3)),
           "point inside the rect");
  QT_END();
}
