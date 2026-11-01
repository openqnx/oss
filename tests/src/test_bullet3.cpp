/* Bullet Physics: known answers from a small rigid-body world -- a sphere
 * (radius 0.5) dropped from 5 m onto a static ground plane comes to rest at
 * height 0.5 and goes to sleep; free fall matches 1/2 g t^2 while it falls;
 * a ray cast hits the ground at y = 0 with the plane's normal; the
 * collision world reports contacts for overlapping objects and none for
 * separated ones; the closest-points query gives the gap between two
 * spheres; LinearMath's quaternion and transform algebra. */
#include "common.h"
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkPairDetector.h>
#include <BulletCollision/NarrowPhaseCollision/btPointCollector.h>
#include <BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.h>
#include <BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.h>
#include <cmath>

namespace {
struct ContactCount : btCollisionWorld::ContactResultCallback {
  int n = 0; btScalar deepest = 0;
  btScalar addSingleResult(btManifoldPoint &cp, const btCollisionObjectWrapper *, int, int,
                           const btCollisionObjectWrapper *, int, int) override {
    ++n; if (cp.getDistance() < deepest) deepest = cp.getDistance();
    return 0;
  }
};
}

int main() {
  QT_BEGIN("bullet3");
  printf("  info Bullet %d (%s precision)\n", btGetVersion(), sizeof(btScalar) == 8 ? "double" : "single");

  /* ---- LinearMath ---- */
  btQuaternion q(btVector3(0, 0, 1), SIMD_HALF_PI);
  btVector3 r = quatRotate(q, btVector3(1, 0, 0));
  QT_CHECK(r.distance(btVector3(0, 1, 0)) < 1e-6, "quaternion: 90 deg about z maps x to y");
  btTransform T(q, btVector3(1, 2, 3));
  QT_CHECK((T.inverse() * (T * btVector3(4, 5, 6))).distance(btVector3(4, 5, 6)) < 1e-5,
           "transform inverse undoes the transform");
  QT_CHECK(std::fabs(btVector3(1, 2, 3).dot(btVector3(4, -5, 6)) - 12) < 1e-6 &&
           btVector3(1, 0, 0).cross(btVector3(0, 1, 0)).distance(btVector3(0, 0, 1)) < 1e-6,
           "dot and cross products");

  /* ---- a falling sphere ---- */
  btDefaultCollisionConfiguration config;
  btCollisionDispatcher dispatcher(&config);
  btDbvtBroadphase broadphase;
  btSequentialImpulseConstraintSolver solver;
  btDiscreteDynamicsWorld world(&dispatcher, &broadphase, &solver, &config);
  world.setGravity(btVector3(0, -9.81f, 0));

  btStaticPlaneShape groundShape(btVector3(0, 1, 0), 0);
  btDefaultMotionState groundMotion;
  btRigidBody ground(btRigidBody::btRigidBodyConstructionInfo(0, &groundMotion, &groundShape));
  ground.setRestitution(0); ground.setFriction(0.8f);
  world.addRigidBody(&ground);

  btSphereShape ballShape(0.5f);
  btVector3 inertia; ballShape.calculateLocalInertia(1, inertia);
  QT_CHECK(std::fabs(inertia.x() - 0.4f * 1 * 0.25f) < 1e-6, "sphere inertia is 2/5 m r^2");
  btDefaultMotionState ballMotion(btTransform(btQuaternion::getIdentity(), btVector3(0, 5, 0)));
  btRigidBody::btRigidBodyConstructionInfo bi(1, &ballMotion, &ballShape, inertia);
  bi.m_restitution = 0; bi.m_friction = 0.8f; bi.m_linearDamping = 0.05f; bi.m_angularDamping = 0.2f;
  btRigidBody ball(bi);
  world.addRigidBody(&ball);

  const btScalar dt = 1.0f / 240;
  /* 0.5 s of free fall: y = 5 - g t^2 / 2 = 3.774 (damping and the
     integrator make it a little higher). */
  for (int i = 0; i < 120; ++i) world.stepSimulation(dt, 0);
  btScalar y05 = ball.getCenterOfMassPosition().y();
  printf("  info height after 0.5 s: %.4f (free fall 3.7738)\n", (double)y05);
  QT_CHECK(std::fabs(y05 - 3.7738f) < 0.05f, "free fall follows 1/2 g t^2");
  QT_CHECK(ball.getLinearVelocity().y() < -4.5f && ball.getLinearVelocity().y() > -4.95f,
           "velocity after 0.5 s is about -g t");

  int steps = 0;
  for (; steps < 240 * 10 && ball.isActive(); ++steps) world.stepSimulation(dt, 0);
  btScalar yRest = ball.getCenterOfMassPosition().y();
  printf("  info rest height %.4f after %.2f s more; state %d\n", (double)yRest, steps * (double)dt,
         ball.getActivationState());
  QT_CHECK(std::fabs(yRest - 0.5f) < 0.02f, "the sphere rests on the plane at its radius (0.5)");
  QT_CHECK(!ball.isActive(), "the resting sphere is put to sleep");
  QT_CHECK(ball.getLinearVelocity().length() < 1e-3, "and is not moving");
  QT_CHECK(std::fabs(ball.getCenterOfMassPosition().x()) < 1e-3 &&
           std::fabs(ball.getCenterOfMassPosition().z()) < 1e-3, "a vertical drop stays vertical");

  /* ---- queries ---- */
  btCollisionWorld::ClosestRayResultCallback ray(btVector3(2, 10, -3), btVector3(2, -10, -3));
  world.rayTest(ray.m_rayFromWorld, ray.m_rayToWorld, ray);
  QT_CHECK(ray.hasHit() && ray.m_collisionObject == &ground &&
           std::fabs(ray.m_hitPointWorld.y()) < 1e-4 && ray.m_hitNormalWorld.distance(btVector3(0, 1, 0)) < 1e-4,
           "ray cast hits the ground at y = 0 with normal +y");
  btCollisionWorld::ClosestRayResultCallback ray2(btVector3(0, 10, 0), btVector3(0, -10, 0));
  world.rayTest(ray2.m_rayFromWorld, ray2.m_rayToWorld, ray2);
  QT_CHECK(ray2.hasHit() && ray2.m_collisionObject == &ball && std::fabs(ray2.m_hitPointWorld.y() - 1.0f) < 0.02f,
           "a ray from above hits the resting sphere's top (y = 1)");

  btBoxShape boxShape(btVector3(1, 1, 1));
  btCollisionObject box; box.setCollisionShape(&boxShape);
  box.setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(0, 1.2f, 0)));
  ContactCount cc; world.contactPairTest(&ball, &box, cc);
  QT_CHECK(cc.n > 0 && cc.deepest < -0.2f, "contactPairTest: sphere and overlapping box are in contact");
  box.setWorldTransform(btTransform(btQuaternion::getIdentity(), btVector3(5, 1.2f, 0)));
  ContactCount cc2; world.contactPairTest(&ball, &box, cc2);
  QT_CHECK(cc2.n == 0, "contactPairTest: separated objects have no contact");

  /* GJK closest points between two unit spheres 3 apart: gap 1. */
  btSphereShape s1(1), s2(1);
  btVoronoiSimplexSolver simplex; btGjkEpaPenetrationDepthSolver epa;
  btGjkPairDetector gjk(&s1, &s2, &simplex, &epa);
  btGjkPairDetector::ClosestPointInput in;
  in.m_transformA = btTransform(btQuaternion::getIdentity(), btVector3(0, 0, 0));
  in.m_transformB = btTransform(btQuaternion::getIdentity(), btVector3(3, 0, 0));
  btPointCollector pc;
  gjk.getClosestPoints(in, pc, nullptr);
  printf("  info GJK distance %.5f\n", (double)pc.m_distance);
  QT_CHECK(pc.m_hasResult && std::fabs(pc.m_distance - 1) < 1e-3, "GJK: distance between spheres 3 apart is 1");

  world.removeRigidBody(&ball);
  world.removeRigidBody(&ground);
  QT_END();
}
