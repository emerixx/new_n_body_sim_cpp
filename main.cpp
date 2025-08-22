#include "global_variables.hpp"
#include "rlgl.h"
#include "vctr.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <string>

using json = nlohmann::json;
const int number_of_bodies =
    3; // has to be const  so i can create arrays of the size
double const_time_step;
double output_to_file_per_computation;
double steps_to_compute;
const double pi = global.pi;
const double G = global.G;
Camera camera;
Image img;
Texture texture;
double B[7][6];
double CH[6] = {16.0 / 135,      0,         6656.0 / 12825,
                28561.0 / 56430, -9.0 / 50, 2.0 / 55};
struct Ds {
  vctr v;
  vctr a;
  Ds() : v(), a() {};
  Ds(vctr aa, vctr b) : v(aa), a(b) {};
  void print() {
    std::cout << "velocity: " << v.output_str()
              << "\nacceleration: " << a.output_str() << std::endl;
  }
};
struct Bs {

  vctr pos;
  vctr v;
  double m;
  Ds ds;
  Bs() : pos(), v(), m(1) {};
  Bs(vctr a, vctr b) : pos(a), v(b), m(1) {};
  Bs(vctr a, vctr b, double c) : pos(a), v(b), m(c) {};
  void print() {
    std::cout << "position: " << pos.output_str()
              << "\nvelocity: " << v.output_str() << "\nmass: " << m
              << std::endl;
  }
};
// delta state  = body state * time
using Bss = std::array<Bs, number_of_bodies>;
using Dss = std::array<Ds, number_of_bodies>;
Bss dss_times_t(Dss dss, double t) {
  Bss out;
  for (int i = 0; i < number_of_bodies; i++) {
    out[i] = Bs(dss[i].v * t, dss[i].a * t);
  }
  return out;
}
Dss dstates_func(Bss bss) {
  // bss(t=0)+dss=bss(t=dt)
  // delta states function
  Dss dss;
  double d_mag;
  std::array<double, number_of_bodies> f_mag;
  vctr d, f;
  for (int i = 0; i < number_of_bodies; i++) {
    f_mag[i] = 0;
  }
  for (int i = 0; i < number_of_bodies; i++) {
    dss[i].a = {0, 0, 0};
    f = vctr();
    for (int j = 0; j < number_of_bodies; j++) {
      if (i == j) {
        continue;
      }
      d = bss[i].pos - bss[j].pos;
      d_mag = d.magnitude();
      if ((d_mag > -0.005 || d_mag < 0.005)) {
        f_mag[i] = G * bss[j].m / pow(d_mag, 2);
      }
      f = f - f_mag[i] * d.normalised();
    }
    dss[i].a = f / bss[i].m;
    dss[i].v = bss[i].v;
  }
  return dss;
}
Bss operator*(Bss bss, double num) {
  Bss out;
  for (int i = 0; i < bss.size(); i++) {
    out[i] = Bs(bss[i].pos * num, bss[i].v * num, bss[i].m);
  }
  return out;
}
Dss operator*(Dss dss, double num) {
  Dss out;
  for (int i = 0; i < dss.size(); i++) {
    out[i] = Ds(dss[i].v * num, dss[i].a * num);
  }
  return out;
}
Dss operator+(Dss dss0, Dss dss1) {
  Dss out;
  for (int i = 0; i < dss0.size(); i++) {
    out[i] = Ds(dss0[i].v + dss1[i].v, dss0[i].a + dss1[i].a);
  }
  return out;
}
Bss operator+(Bss bss0, Bss bss1) {
  Bss out;
  for (int i = 0; i < bss0.size(); i++) {
    out[i] = Bs(bss0[i].pos + bss1[i].pos, bss0[i].v + bss1[i].v, bss0[i].m);
  }
  return out;
}

Bss Euler(Bss bss, double dt) {
  Dss dss = dstates_func(bss);
  for (int i = 0; i < number_of_bodies; i++) {
    bss[i].pos = bss[i].pos + bss[i].v * dt;
    bss[i].v = bss[i].v + dss[i].a * dt;
  }
  return bss;
}
Bss RK4(Bss bss, double dt) {
  Dss k[4];
  k[0] = dstates_func(bss);
  k[1] = dstates_func(bss + dss_times_t(k[0], dt / 2));
  k[2] = dstates_func(bss + dss_times_t(k[1], dt / 2));
  k[3] = dstates_func(bss + dss_times_t(k[2], dt));
  bss = bss + dss_times_t(k[0] + k[1] * 2 + k[2] * 2 + k[3], dt / 6);
  return bss;
}
Bss RKF45(Bss bss, double dt) {
  Bss k[6];
  Dss dss[6];
  dss[0] = dstates_func(bss);
  dss[1] = dstates_func(bss + k[0] * B[2][1]);
  dss[2] = dstates_func(bss + k[0] * B[3][1] + k[1] * B[3][2]);
  dss[3] = dstates_func(bss + k[0] * B[4][1] + k[1] * B[4][2] + k[2] * B[4][3]);
  dss[4] = dstates_func(bss + k[0] * B[5][1] + k[1] * B[5][2] + k[2] * B[5][3] +
                        k[3] * B[5][4]);
  dss[5] = dstates_func(bss + k[0] * B[6][1] + k[1] * B[6][2] + k[2] * B[6][3] +
                        k[3] * B[6][4] + k[4] * B[6][5]);
  for (int i = 0; i < 6; i++) {
    k[i] = dss_times_t(dss[i], dt);
  }
  bss = bss + k[0] * CH[0] + k[1] * CH[1] + k[2] * CH[2] + k[3] * CH[3] +
        k[4] * CH[4] + k[5] * CH[5];
  return bss;
}
Bss update(Bss bss, double dt) {
  bss = RKF45(bss, dt);
  // bss = RK4(bss, dt);
  //  bss = Euler(bss, dt);
  return bss;
}
void saveBss(Bss bss, std::ofstream fo[]) {
  for (int i = 0; i < bss.size(); i++) {
    fo[i] << bss[i].pos.output_str() + "\n";
  }
}
void printBss(Bss bss) {
  for (int i = 0; i < bss.size(); i++) {
    std::cout << i << "{\n";
    bss[i].print();
    std::cout << "};\n";
  }
}
Bss load_body_setup_from_file() {
  Bss bss;
  std::ifstream fin(global.setup_dir + "body_setup.json");
  if (fin) {
    std::cout << "Body setup file opened successfully\n";
  } else {
    std::cerr << "Couldnt open body setup file, exiting...\n";
    exit(-1);
  }
  json j;
  fin >> j;
  fin.close();
  Bs bs;
  for (int i = 0; i < number_of_bodies; i++) {
    bs.pos.x = j["body" + std::to_string(i)]["pos"][0];
    bs.pos.y = j["body" + std::to_string(i)]["pos"][1];
    bs.pos.z = j["body" + std::to_string(i)]["pos"][2];
    bs.v.x = j["body" + std::to_string(i)]["v"][0];
    bs.v.y = j["body" + std::to_string(i)]["v"][1];
    bs.v.z = j["body" + std::to_string(i)]["v"][2];
    bs.m = j["body" + std::to_string(i)]["m"];
    bss[i] = bs;
  }
  return bss;
}
void load_settings_from_file() {
  std::ifstream fin(global.setup_dir + "settings.json");
  if (fin) {
    std::cout << "Settings file opened successfully\n";
  } else {
    std::cerr << "Couldnt open settings file, exiting...\n";
    exit(-1);
  }
  json j;
  int pwr;
  fin >> j;
  fin.close();
  pwr = j["computation"]["constant_time_step_pwr"];
  const_time_step = pow(10, pwr);
  pwr = j["computation"]["output_to_file_per_computation_pwr"];
  output_to_file_per_computation = pow(10, pwr);
  pwr = j["computation"]["steps_to_compute_pwr"];
  steps_to_compute = pow(10, pwr);
}
int main() {
  std::cout << "Start\n";
  B[2][1] = 1.0 / 4.0;
  B[3][1] = 3.0 / 32;
  B[3][2] = 9.0 / 32;
  B[4][1] = 1932.0 / 2197;
  B[4][2] = -7200.0 / 2197;
  B[4][3] = 7296.0 / 2197;
  B[5][1] = 439.0 / 216;
  B[5][2] = -8;
  B[5][3] = 3680.0 / 513;
  B[5][4] = -845.0 / 4104;
  B[6][1] = -8.0 / 27;
  B[6][2] = 2;
  B[6][3] = -3544.0 / 2565;
  B[6][4] = 1859.0 / 4104;
  B[6][5] = -11.0 / 40;
  Bss bss = load_body_setup_from_file();
  load_settings_from_file();
  std::ofstream files_out[number_of_bodies];
  std::string input;
  int exp;
  std::cout << "Current settings:\n";
  std::cout << "Constant time step: 10^" << log10(const_time_step) << std::endl;
  std::cout
      << "Number of lines in the output file after computation: 10^"
      << log10(output_to_file_per_computation) << std::endl;
  std::cout << "Steps to compute: 10^" << log10(steps_to_compute) << std::endl;
  std::cout << "Do u want to edit these settings? (y/N)\n";
  std::getline(std::cin, input); // Read entire line including spaces
  if (input.empty()) {
    std::cout << "Using default" << std::endl;
  } else {
    // Steps to compute START
    std::cout << "Steps to compute, (10^n), default 10^" << log10(steps_to_compute) << ": ";
    std::getline(std::cin, input); // Read entire line including spaces
    if (input.empty()) {
      std::cout << "Using default" << std::endl;
    } else {
      std::cout << "Using 10^" << input << std::endl;
      exp = std::stoi(input);
      steps_to_compute = pow(10, exp);
    }
    // Steps to compute END
    input = "";
    // Constant time step START
    std::cout << "Constant time step, (10^-n), default 10^" << log10(const_time_step)
              << ": ";
    std::getline(std::cin, input); // Read entire line including spaces
    if (input.empty()) {
      std::cout << "Using default" << std::endl;
    } else {
      std::cout << "Using 10^-" << input << std::endl;
      exp = std::stoi(input);
      const_time_step = pow(10, -exp);
    }
    // Constant time step END
    input = "";
    // Times to output to file START
    std::cout << "Times to output to file, (10^n), default 10^" << log10(output_to_file_per_computation)
              << ": ";
    std::getline(std::cin, input); // Read entire line including spaces
    if (input.empty()) {
      std::cout << "Using default" << std::endl;
    } else {
      std::cout << "Using 10^" << input << std::endl;
      exp = std::stoi(input);
      output_to_file_per_computation = pow(10, exp);
    }
    // Times to output to file END
  }
  for (int i = 0; i < number_of_bodies; ++i) {
    files_out[i].open(global.output_dir + "body" + std::to_string(i));
    if (!files_out[i]) {
      std::cerr << "Failed to open "
                << global.output_dir + "body" + std::to_string(i) << std::endl;
      return 1;
    }
  }
  int x[2] = {0, 0};
  for (double i = 0; i <= steps_to_compute; i++) {
    bss = update(bss, const_time_step);
    if (x[0] == steps_to_compute / output_to_file_per_computation) {
      x[0] = 0;
      saveBss(bss, files_out);
    }
    if (x[1] == steps_to_compute / 100) {
      x[1] = 0;
      std::cout << i / steps_to_compute * 100 << "%\n";
    }
    x[0]++;
    x[1]++;
  }
  printBss(bss);
  for (int i = 0; i < number_of_bodies; i++) {
    files_out[i].close();
  }

  return 0;
}
