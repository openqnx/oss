// GLM: matrix/vector math.
#include "common.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
int main() {
  QT_BEGIN("glm");
  glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(1, 2, 3));
  glm::vec4 p = t * glm::vec4(0, 0, 0, 1);
  QT_CHECK(p.x == 1 && p.y == 2 && p.z == 3, "translate");
  glm::mat4 r = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
  glm::vec4 q = r * glm::vec4(1, 0, 0, 1);
  QT_CHECK(std::fabs(q.y - 1.0f) < 1e-5f, "rotate");
  QT_CHECK(glm::dot(glm::vec3(1, 0, 0), glm::vec3(0, 1, 0)) == 0.0f, "dot");
  QT_END();
}
