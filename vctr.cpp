#include "vctr.hpp"
#include <cmath>
#include <iostream>
#include <string>

vctr operator-(vctr a, vctr b) { return vctr(a.x - b.x, a.y - b.y, a.z - b.z); }

vctr operator+(vctr a, vctr b) { return vctr(a.x + b.x, a.y + b.y, a.z + b.z); }

vctr operator/(vctr a, double b) { return vctr(a.x / b, a.y / b, a.z / b); }

vctr operator*(vctr a, double b) { return vctr(a.x * b, a.y * b, a.z * b); }
vctr operator*(double a, vctr b) { return vctr(b.x * a, b.y * a, b.z * a); }

vctrf vecToVecf(vctr a) { return vctrf(a.x, a.y, a.z); }

double vctr::magnitude() { return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2)); };

vctr vctr::normalised() {
  double mag = magnitude();
  return vctr(x / mag, y / mag, z / mag);
}
vctr vctr::opposite() { return vctr(-x, -y, -z); }
void print(vctr a) {
  std::cout << "vec.x: " + std::to_string(a.x) << "\n";
  std::cout << "vec.y: " + std::to_string(a.y) << "\n";
  std::cout << "vec.z: " + std::to_string(a.z) << "\n";
}

std::string vctr::output_str() {
  return "{" + std::to_string(this->x) + "," + std::to_string(this->y) +
         "," + std::to_string(this->z) + "}";
}
