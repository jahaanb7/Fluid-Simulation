#include <glm/glm.hpp>
#include <cmath>

class Quaternion{
  public:
    float w, x, y, z;

    Quaternion(float w1 = 1.0f, float x1 = 0.0f, float y1 = 0.0f, float z1 = 0.0f)
        : w(w1), x(x1), y(y1), z(z1){}

    Quaternion convertVector(glm::vec3 position, float real){
      x = position.x;
      y = position.y;
      z = position.z;
      w = real;

      return Quaternion(x,y,z,w);
    }

    Quaternion inverse(){
      float magnitude = (w * w) + (x*x) + (y*y) + (z*z);

      return Quaternion(w, -x/magnitude, -y/magnitude, -z/magnitude);
    }

    Quaternion operator*(const Quaternion& q) const {
        return Quaternion(
            w*q.w - x*q.x - y*q.y - z*q.z,
            w*q.x + x*q.w + y*q.z - z*q.y,
            w*q.y - x*q.z + y*q.w + z*q.x,
            w*q.z + x*q.y - y*q.x + z*q.w
        );
    }

    Quaternion conjugate() const {
        return Quaternion(w, -x, -y, -z);
    }

    glm::vec3 rotate(const glm::vec3& v) const {

        Quaternion p(0, v.x, v.y, v.z);

        Quaternion result = (*this) * p * conjugate();

        return glm::vec3(result.x, result.y, result.z);
    }

    static Quaternion quaternionRotation(glm::vec3 axis, float theta){
      float half_angle = (glm::radians(theta))/2;

      float cos = glm::cos(half_angle);
      float sin = glm::sin(half_angle);

      axis = glm::normalize(axis);

      return Quaternion(
        cos,
        sin * axis.x,
        sin * axis.y,
        sin * axis.z
      );
    }

    void normalize() {
      float norm = sqrt(x*x + y*y + z*z + w*w);
      if (norm > 0.0001){
          w /= norm;
          x /= norm;
          y /= norm;
          z /= norm;
      }
    }
};