#include "global_variables.hpp"
#include "rlgl.h"
#include "vctr.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
const int n_of_bodies = 2;
const double pi = global.pi;
const double G = global.G;
const bool output_to_file = 1;
const bool output_to_window = 0;
const Color clrs[] = {RED, BLUE, GREEN};
Camera camera;
Image img;
Texture texture;
bool is_stopped = false;

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

Bs update_bs(Bs bs) {
  double dt = global.constTimeStep * GetFrameTime() * global.speed;
  // std::cout << "dt: " << dt << std::endl;
  bs.pos = bs.pos + bs.v * dt;
  bs.v = bs.v + bs.ds.a * dt;
  return bs;
}

Bss update(Bss bss) {
  double d_mag;
  std::array<double, n_of_bodies> f_mag;
  vctr d;
  vctr f;
  for (int i = 0; i < n_of_bodies; i++) {
    f_mag[i] = 0;
  }
  for (int i = 0; i < n_of_bodies; i++) {
    bss[i].ds.a = {0, 0, 0};
    f = vctr();
    for (int j = 0; j < n_of_bodies; j++) {
      if (i == j)
        continue;
      // std::cout << "i|j: " << i << "|" << j << std::endl;

      // if i and j werent run b4

      d = (bss[i].pos - bss[j].pos);
      d_mag = d.magnitude();

      //-0.005<d.x<0.005
      if ((d_mag > -0.005 || d_mag < 0.005))
        f_mag[i] = G * bss[j].m / pow(d_mag, 2);

      f = f - f_mag[i] * d.normalised();
      // std::cout << "f_mag: " << f_mag[i] << std::endl;
      // std::cout << "d.norm: " << d.normalised().output_str() << std::endl;
      //  {G*bss[j].m/pow(d.x,2),G*bss[j].m/pow(d.y,2),G*bss[j].m/pow(d.z,2)};
    }
    // update acc
    // std::cout << "force: " << f.output_str() << "; i:" << i << std::endl;

    bss[i].ds.a = f / bss[i].m;
    bss[i] = update_bs(bss[i]);
  }
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
  if (output_to_window) {
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
  std::ofstream files_out[n_of_bodies];

  for (int i = 0; i < n_of_bodies; ++i) {
    files_out[i].open(global.output_dir + "body" + std::to_string(i));
    if (!files_out[i]) {
      std::cerr << "Failed to open "
                << global.output_dir + "body" + std::to_string(i) << std::endl;
      return 1;
    }
  }
  int x = 0;
  if (output_to_window) {
    while (!WindowShouldClose()) {
      bss = update(bss);
      if (output_to_window) {
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
      if (x == global.output_to_file_every_n_steps && output_to_file) {
        x = 0;
        saveBss(bss, files_out);
      }
      x++;
    }
    for (int i; i < n_of_bodies; i++) {
      files_out[i].close();
    }
  }
  if (output_to_file && !output_to_window) {
    std::cout << "Outputing to files every "
              << global.output_to_file_every_n_steps << " steps\n";
    std::cout << "Time to compute, (10^n): ";
    std::string input;
    std::cin >> input;
    int exp = std::stoi(input);
    double time_to_compute = pow(10, exp);
    int x = 0;
    for (double i = 0; i < time_to_compute; i++) {
      bss = update(bss);
      if (x == global.output_to_file_every_n_steps) {
        x=0;
        saveBss(bss, files_out);
      }
      x++;
    }
  }
  return 0;
}
