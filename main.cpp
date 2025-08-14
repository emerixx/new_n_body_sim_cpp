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
const int n_of_bodies = 2;
const double pi = global.pi;
const double G = global.G;
const bool output_to_file = 1;
const Color clrs[] = {RED, BLUE, GREEN};
Camera camera;
Image img;
Texture texture;
bool is_stopped = false;

double B[7][6];

double CH[6] = {16.0/135, 0, 6656.0/12825, 28561.0/56430, -9.0/50, 2.0/55};

void drawGrid(int slices, float spacing, Color color) {

  int halfSlices = slices / 2;

  rlBegin(RL_LINES);
  for (int i = -halfSlices; i <= halfSlices; i++) {

    rlColor3f(color.r / 255.f, color.g / 255.f, color.b / 255.f);

    rlVertex3f((float)i * spacing, 0.0f, (float)-halfSlices * spacing);
    rlVertex3f((float)i * spacing, 0.0f, (float)halfSlices * spacing);

    rlVertex3f((float)-halfSlices * spacing, 0.0f, (float)i * spacing);
    rlVertex3f((float)halfSlices * spacing, 0.0f, (float)i * spacing);
  }
  rlEnd();
}
void drawCircle(vctr centerOffset, float radius, Color color, float scale) {
  int segments = global.circleSegments;
  double step = 2 * pi / segments;
  double angle = 0;
  vctr center = {centerOffset.x * scale, centerOffset.y * scale};
  vctr vertex2 = {sinf(angle + step) * (float)radius,
                  cosf(angle + step) * (float)radius};
  vctr vertex2old = {0, (float)radius};
  rlBegin(RL_TRIANGLES);
  rlSetTexture(texture.id);
  for (int i = 0; i < segments; i++) {

    rlColor4ub(color.r, color.g, color.b, color.a);
    rlVertex2f(center.x, center.y);
    rlVertex2f(vertex2.x + center.x, vertex2.y + center.y);
    rlVertex2f(vertex2old.x + center.x, vertex2old.y + center.y);
    angle += step;
    vertex2old = vertex2;
    vertex2 = {sinf(angle + step) * (float)radius,
               cosf(angle + step) * (float)radius};
  }
  rlEnd();
}
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

// delta state
// body state * time
using Bss = std::array<Bs, n_of_bodies>;
using Dss = std::array<Ds, n_of_bodies>;

Bss dss_times_t(Dss dss, double t) {

  Bss out;
  for (int i = 0; i < n_of_bodies; i++) {

    out[i] = Bs(dss[i].v *t, dss[i].a *t);
  }
  return out;
}

// bss(t=0)+dss=bss(t=dt)
// delta states function
Dss dstates_func(Bss bss) {
  Dss dss;
  double d_mag;
  std::array<double, n_of_bodies> f_mag;
  vctr d;
  vctr f;
  for (int i = 0; i < n_of_bodies; i++) {
    f_mag[i] = 0;
  }
  for (int i = 0; i < n_of_bodies; i++) {
    dss[i].a = {0, 0, 0};
    f = vctr();
    for (int j = 0; j < n_of_bodies; j++) {
      if (i == j)
        continue;

      d = (bss[i].pos - bss[j].pos);
      d_mag = d.magnitude();

      if ((d_mag > -0.005 || d_mag < 0.005))
        f_mag[i] = G * bss[j].m / pow(d_mag, 2);

      f = f - f_mag[i] * d.normalised();
    }

    dss[i].a = f / bss[i].m;
    dss[i].v = bss[i].v;
  }
  return dss;
}

Bss operator*(Bss bss, double num) {
  Bss out;
  for (int i = 0; i < n_of_bodies; i++) {
    out[i] = Bs(bss[i].pos * num, bss[i].v * num, bss[i].m);
  }
  return out;
}
Dss operator*(Dss dss, double num) {
  Dss out;
  for (int i = 0; i < n_of_bodies; i++) {
    out[i] = Ds(dss[i].v * num, dss[i].a * num);
  }
  return out;
}
Dss operator+(Dss dss0, Dss dss1) {
  Dss out;
  for (int i = 0; i < n_of_bodies; i++) {
    out[i] = Ds(dss0[i].v + dss1[i].v, dss0[i].a + dss1[i].a);
  }
  return out;
}
Bss operator+(Bss bss0, Bss bss1) {
  Bss out;
  for (int i = 0; i < n_of_bodies; i++) {
    out[i] = Bs(bss0[i].pos +bss1[i].pos, bss0[i].v + bss1[i].v, bss0[i].m);
  }
  return out;
}

Bss Euler(Bss bss, double dt) {
  Dss dss = dstates_func(bss);
  for (int i = 0; i < n_of_bodies; i++) {

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

Bss update(Bss bss) {

  double dt = global.constTimeStep * global.speed;
  if (!output_to_file) {
    dt = dt * GetFrameTime();
  }

  bss = RKF45(bss, dt);
  //bss = RK4(bss, dt);
  // bss = Euler(bss, dt);
  return bss;
}
void drawBss(Bss bss) {
  for (int i = 0; i < n_of_bodies; i++) {
    drawCircle(bss[i].pos, global.circle_radius, clrs[i], global.drawScale);
  }
}
void saveBss(Bss bss, std::ofstream fo[]) {
  for (int i = 0; i < n_of_bodies; i++) {
    fo[i] << bss[i].pos.output_str() + "\n";
  }
}
void printBss(Bss bss) {
  for (int i = 0; i < n_of_bodies; i++) {
    std::cout << i << "{\n";
    bss[i].print();
    std::cout << "};\n";
  }
}
int main() {
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
  if (!output_to_file) {
    InitWindow(global.winSize.x, global.winSize.y, "meow");
    camera.position = global.camera_startPos;

    camera.target = global.camera_startTarget;
    camera.up = global.camera_up;
    camera.fovy = global.camera_startFovy;
    camera.projection = CAMERA_PERSPECTIVE;

    Image img = GenImageColor(32, 32, WHITE);
    Texture texture = LoadTextureFromImage(img);
    UnloadImage(img);
  }
  Bss bss;
  std::ifstream fin(global.setup_dir + "body_setup.json");
  json j;
  fin >> j;
  Bs bs;
  for (int i = 0; i < n_of_bodies; i++) {
    bs.pos.x = j["body" + std::to_string(i)]["pos"][0];
    bs.pos.y = j["body" + std::to_string(i)]["pos"][1];
    bs.pos.z = j["body" + std::to_string(i)]["pos"][2];
    bs.v.x = j["body" + std::to_string(i)]["v"][0];
    bs.v.y = j["body" + std::to_string(i)]["v"][1];
    bs.v.z = j["body" + std::to_string(i)]["v"][2];
    bs.m = j["body" + std::to_string(i)]["m"];
    bss[i] = bs;
  }
  fin.close();
  if (!output_to_file) {
    while (!WindowShouldClose()) {
      bss = update(bss);
      BeginDrawing();
      ClearBackground(BLACK);
      DrawFPS(0, 0);
      BeginMode3D(camera);
      rlPushMatrix();
      rlRotatef(90, 1, 0, 0);
      drawGrid(global.grid_lines, global.grid_spacing, WHITE);
      rlPopMatrix();
      drawBss(bss);
      EndMode3D();
      EndDrawing();
    }
  }
  if (output_to_file) {
    double otfpc = global.output_to_file_per_computation;
    std::ofstream files_out[n_of_bodies];

    for (int i = 0; i < n_of_bodies; ++i) {
      files_out[i].open(global.output_dir + "body" + std::to_string(i));
      if (!files_out[i]) {
        std::cerr << "Failed to open "
                  << global.output_dir + "body" + std::to_string(i)
                  << std::endl;
        return 1;
      }
    }
    std::cout << "Time step: " << global.constTimeStep << std::endl;
    std::cout << "Time to compute, (10^n): ";
    std::string input;
    std::cin >> input;
    int exp = std::stoi(input);
    double time_to_compute = pow(10, exp);
    std::cout << "Times to output to file, (10^n), default 10^" << log10(otfpc)
              << ": " << std::endl;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::getline(std::cin, input); // Read entire line including spaces
    if (input.empty()) {
      std::cout << "Using default" << std::endl;
    } else {
      exp = std::stoi(input);
      otfpc = pow(10, exp);
    }

    int x[2] = {0, 0};
    for (double i = 0; i <= time_to_compute; i++) {
      bss = update(bss);
      if (x[0] == time_to_compute / otfpc) {
        x[0] = 0;
        saveBss(bss, files_out);
      }
      if (x[1] == time_to_compute / 100) {
        x[1] = 0;
        std::cout << i / time_to_compute * 100 << "%\n";
      }
      x[0]++;
      x[1]++;
    }
    printBss(bss);
    for (int i = 0; i < n_of_bodies; i++) {
      files_out[i].close();
    }
  }
  return 0;
}
