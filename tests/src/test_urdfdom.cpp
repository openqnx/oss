// urdfdom: parse a small robot description (URDF) -- three links, a
// revolute joint with limits and dynamics, a prismatic joint, a fixed
// joint, materials, inertia, visual and collision geometry -- and check the
// model it builds: the tree (root, parents, children), joint types, axes,
// origins (rpy -> quaternion), limits, a mesh's scale, a material colour; a
// file read with parseURDFFile, and malformed input rejected.
#include "common.h"
#include <urdf_parser/urdf_parser.h>
#include <cmath>
#include <cstdlib>
#include <string>
#include <unistd.h>

static bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) < eps; }

static const char *kRobot = R"(<?xml version="1.0"?>
<robot name="qnx_arm">
  <material name="blue"><color rgba="0 0 0.8 1"/></material>
  <link name="base_link">
    <inertial>
      <origin xyz="0 0 0.05"/>
      <mass value="2.5"/>
      <inertia ixx="0.01" ixy="0" ixz="0" iyy="0.02" iyz="0" izz="0.03"/>
    </inertial>
    <visual><geometry><cylinder radius="0.1" length="0.1"/></geometry><material name="blue"/></visual>
    <collision><geometry><cylinder radius="0.1" length="0.1"/></geometry></collision>
  </link>
  <link name="upper_arm">
    <visual><geometry><box size="0.05 0.05 0.4"/></geometry></visual>
    <collision><geometry><mesh filename="package://qnx_arm/upper.stl" scale="0.001 0.001 0.001"/></geometry></collision>
  </link>
  <link name="slider"><visual><geometry><sphere radius="0.03"/></geometry></visual></link>
  <link name="tool"/>
  <joint name="shoulder" type="revolute">
    <parent link="base_link"/><child link="upper_arm"/>
    <origin xyz="0 0 0.1" rpy="0 0 1.5707963267948966"/>
    <axis xyz="0 1 0"/>
    <limit lower="-1.57" upper="1.57" effort="30" velocity="2.0"/>
    <dynamics damping="0.7" friction="0.1"/>
  </joint>
  <joint name="extend" type="prismatic">
    <parent link="upper_arm"/><child link="slider"/>
    <origin xyz="0 0 0.4"/>
    <axis xyz="0 0 1"/>
    <limit lower="0" upper="0.25" effort="100" velocity="0.5"/>
  </joint>
  <joint name="tool_mount" type="fixed">
    <parent link="slider"/><child link="tool"/>
  </joint>
</robot>
)";

static void checkModel(const urdf::ModelInterfaceSharedPtr &m, const char *tag) {
  std::string t(tag);
  QT_CHECK(m && m->getName() == "qnx_arm", (t + ": robot name").c_str());
  if (!m) return;
  QT_CHECK(m->links_.size() == 4 && m->joints_.size() == 3, (t + ": 4 links, 3 joints").c_str());
  urdf::LinkConstSharedPtr root = m->getRoot();
  QT_CHECK(root && root->name == "base_link" && root->child_links.size() == 1 &&
           root->child_links[0]->name == "upper_arm", (t + ": root base_link with child upper_arm").c_str());
  urdf::LinkConstSharedPtr tool = m->getLink("tool");
  QT_CHECK(tool && tool->getParent() && tool->getParent()->name == "slider" &&
           tool->parent_joint && tool->parent_joint->type == urdf::Joint::FIXED, (t + ": tool hangs on slider by a fixed joint").c_str());

  urdf::JointConstSharedPtr sh = m->getJoint("shoulder");
  bool ok = sh && sh->type == urdf::Joint::REVOLUTE && sh->parent_link_name == "base_link" &&
            sh->child_link_name == "upper_arm" && near(sh->axis.y, 1) && near(sh->axis.x, 0);
  QT_CHECK(ok, (t + ": shoulder is revolute about y").c_str());
  if (sh) {
    const urdf::Pose &p = sh->parent_to_joint_origin_transform;
    double qx, qy, qz, qw; p.rotation.getQuaternion(qx, qy, qz, qw);
    QT_CHECK(near(p.position.z, 0.1) && near(qz, std::sqrt(0.5), 1e-9) && near(qw, std::sqrt(0.5), 1e-9),
             (t + ": shoulder origin z 0.1, yaw 90 deg").c_str());
    QT_CHECK(sh->limits && near(sh->limits->lower, -1.57) && near(sh->limits->upper, 1.57) &&
             near(sh->limits->effort, 30) && near(sh->limits->velocity, 2.0), (t + ": shoulder limits").c_str());
    QT_CHECK(sh->dynamics && near(sh->dynamics->damping, 0.7) && near(sh->dynamics->friction, 0.1),
             (t + ": shoulder dynamics").c_str());
  }
  urdf::JointConstSharedPtr ex = m->getJoint("extend");
  QT_CHECK(ex && ex->type == urdf::Joint::PRISMATIC && ex->limits && near(ex->limits->upper, 0.25) &&
           near(ex->parent_to_joint_origin_transform.position.z, 0.4), (t + ": extend is prismatic, 0..0.25").c_str());

  urdf::LinkConstSharedPtr base = m->getLink("base_link");
  ok = base && base->inertial && near(base->inertial->mass, 2.5) && near(base->inertial->izz, 0.03) &&
       near(base->inertial->origin.position.z, 0.05);
  QT_CHECK(ok, (t + ": base_link inertial (mass 2.5, izz 0.03)").c_str());
  ok = base && base->visual && base->visual->geometry && base->visual->geometry->type == urdf::Geometry::CYLINDER;
  if (ok) {
    auto cyl = std::dynamic_pointer_cast<urdf::Cylinder>(base->visual->geometry);
    ok = cyl && near(cyl->radius, 0.1) && near(cyl->length, 0.1);
  }
  QT_CHECK(ok, (t + ": base_link visual is a cylinder r 0.1 l 0.1").c_str());
  urdf::MaterialSharedPtr blue = m->getMaterial("blue");
  QT_CHECK(blue && near(blue->color.b, 0.8, 1e-6) && near(blue->color.r, 0, 1e-6) && near(blue->color.a, 1, 1e-6),
           (t + ": material blue = rgba 0 0 0.8 1").c_str());
  urdf::LinkConstSharedPtr ua = m->getLink("upper_arm");
  ok = ua && ua->collision && ua->collision->geometry && ua->collision->geometry->type == urdf::Geometry::MESH;
  if (ok) {
    auto mesh = std::dynamic_pointer_cast<urdf::Mesh>(ua->collision->geometry);
    ok = mesh && mesh->filename == "package://qnx_arm/upper.stl" && near(mesh->scale.x, 0.001);
  }
  QT_CHECK(ok, (t + ": upper_arm collision mesh and scale").c_str());
}

int main() {
  QT_BEGIN("urdfdom");
  urdf::ModelInterfaceSharedPtr m = urdf::parseURDF(kRobot);
  checkModel(m, "string");

  char path[] = "/tmp/qnxport-urdf-XXXXXX";
  int fd = mkstemp(path);
  bool wrote = fd >= 0 && write(fd, kRobot, strlen(kRobot)) == (ssize_t)strlen(kRobot);
  if (fd >= 0) close(fd);
  QT_CHECK(wrote, "wrote the URDF to a temporary file");
  urdf::ModelInterfaceSharedPtr mf = urdf::parseURDFFile(path);
  unlink(path);
  QT_CHECK(mf && mf->getRoot() && mf->getRoot()->name == "base_link" && mf->joints_.size() == 3,
           "parseURDFFile reads the same model");

  QT_CHECK(!urdf::parseURDF("<robot name=\"x\"><link name=\"a\"/><joint name=\"j\" type=\"revolute\">"
                            "<parent link=\"a\"/><child link=\"missing\"/></joint></robot>"),
           "a joint naming a missing link is rejected");
  QT_CHECK(!urdf::parseURDF("<robot name=\"x\"><link name=\"a\">"), "malformed XML is rejected");
  QT_CHECK(!urdf::parseURDF("<robot name=\"x\"><link name=\"a\"/><link name=\"b\"/></robot>"),
           "two links and no joint (no single root) is rejected");
  QT_END();
}
