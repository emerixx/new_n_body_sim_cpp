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
const int circle_segments = 10;
const int n_of_bodies = 3;
const double pi = 3.1415926535897932384626433832795028841971693993751;
const double G = 4 * pow(pi, 2); // AU^3 MO^-1 Year^-2
const Color clrs[] = {RED, BLUE, GREEN};
const Vector3 winSize = {1000, 1000};
const Vector3 winCenter = {winSize.x / 2, winSize.y / 2};
Camera camera;
Image img;
Texture texture;
bool is_stopped = false;
  const Vector3 camera_startPos = {0, 0, 250};
  const Vector3 camera_startTarget = {0, 0, 0};
  const Vector3 camera_up = {0, 1, 0};
  const float camera_startFovy = 60;

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
  int segments = circle_segments;
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

// drawCircle(bss[i].pos, global.circle_radius, clrs[i], global.drawScale);

int main() {
  InitWindow(winSize.x, winSize.y, "meow");
  camera.position = camera_startPos;

  camera.target = global.camera_startTarget;
  camera.up = global.camera_up;
  camera.fovy = global.camera_startFovy;
  camera.projection = CAMERA_PERSPECTIVE;

  Image img = GenImageColor(32, 32, WHITE);
  Texture texture = LoadTextureFromImage(img);
  UnloadImage(img);
  Bss bss;
  // Load from file
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
  std::ofstream fo[n_of_bodies];

  for (int i = 0; i < n_of_bodies; ++i) {
    fo[i].open(global.output_dir + "body" + std::to_string(i));
    if (!fo[i]) {
      std::cerr << "Failed to open "
                << global.output_dir + "body" + std::to_string(i) << std::endl;
      return 1;
    }
  }
  int x = 0;
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
    if (x == global.out_to_f_step) {
      x = 0;
      saveBss(bss, fo);
    }
    EndMode3D();
    EndDrawing();
    x++;
  }
  for (int i; i < n_of_bodies; i++)
    fo[i].close();
  return 0;
}
