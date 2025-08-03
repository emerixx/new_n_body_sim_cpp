#ifndef VCTR_H
#define VCTR_H



#include <string>
struct vctr {
  double x;
  double y;
  double z;
  vctr(double x_in, double y_in) : x(x_in), y(y_in), z(0) {}
  vctr(double x_in, double y_in, double z_in) : x(x_in), y(y_in), z(z_in) {}
  vctr() : x(0), y(0), z(0) {}
  std::string output_str();
};

struct vctrf {
  float x;
  float y;
  float z;
  vctrf(float x_in, float y_in) : x(x_in), y(y_in), z(0) {}
  vctrf(float x_in, float y_in, float z_in) : x(x_in), y(y_in), z(z_in) {}
  vctrf() : x(0), y(0), z(0) {}
};

vctr operator-(vctr a, vctr b);
vctr operator+(vctr a, vctr b);
vctr operator/(vctr a, double b);
vctr operator*(vctr a, double b);
vctrf vecToVecf(vctr a);
double magnitude(vctr a);
vctr normalize(vctr a);
vctr opposite(vctr a);
void print(vctr a);

#endif // VCTR_H
