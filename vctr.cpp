#include "vctr.hpp"
#include <cmath>
#include <iostream>
#include <string>

vctr operator-(vctr a, vctr b) { return vctr(a.x - b.x, a.y - b.y, a.z - b.z); }

vctr operator+(vctr a, vctr b) { return vctr(a.x + b.x, a.y + b.y, a.z + b.z); }

vctr operator/(vctr a, double b) { return vctr(a.x / b, a.y / b, a.z / b); }

vctr operator*(vctr a, double b) { return vctr(a.x * b, a.y * b, a.z * b); }

vctrf vecToVecf(vctr a) { return vctrf(a.x, a.y, a.z); }

double vctr::magnitude() {
  return sqrt(pow(x, 2)+pow(y,2)+pow(z,2));
};

vctr normalize(vctr a) {
  double mag = a.magnitude();
  return vctr(a.x / mag, a.y / mag, a.z / mag);
}
vctr opposite(vctr a) { return vctr(-a.x, -a.y, -a.z); }
void print(vctr a) {
  std::cout << "vec.x: " + std::to_string(a.x) << "\n";
  std::cout << "vec.y: " + std::to_string(a.y) << "\n";
  std::cout << "vec.z: " + std::to_string(a.z) << "\n";
}

std::string vctr::output_str() {
  return "{ " + std::to_string(this->x) + " , : " + std::to_string(this->y) +
         " , " + std::to_string(this->z) + " }";
}
