#include "global_variables.hpp"
#include "rlgl.h"
#include "vctr.hpp"
#include <array>
#include <iostream>
#include <raylib.h>

const int nOfBodies = 2;
const double pi = global.pi;
const double G = global.G;
const Color clrs[] = {RED, BLUE};
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

struct bs_struct {

  vctr pos;
  vctr v;
  double m;
  bs_struct() : pos(), v(), m(1) {};
  bs_struct(vctr a, vctr b) : pos(a), v(b), m(1) {};
  bs_struct(vctr a, vctr b, double c) : pos(a), v(b), m(c) {};
  void print() {
    std::cout << "position: " << pos.output_str()
              << "\nvelocity: " << v.output_str() << "\nmass: " << m
              << std::endl;
  }
};

// delta state
// body state * time
struct ds_struct {
  vctr v;
  vctr a;
  ds_struct() : v(), a() {};
  ds_struct(vctr aa, vctr b) : v(aa), a(b) {};
  void print() {
    std::cout << "velocity: " << v.output_str()
              << "\nacceleration: " << a.output_str() << std::endl;
  }
};
using Bss = std::array<bs_struct, nOfBodies>;

bs_struct update_bs(bs_struct bs) {
  double dt = global.constTimeStep * GetFrameTime() * global.speed;
  vctr a;
  




  bs.pos = bs.pos + bs.v * dt;
  bs.v = bs.v + a * dt;
  return bs;
}

Bss update(Bss bss) {
  for (int i = 0; i < nOfBodies; i++) {
    bss[i] = update_bs(bss[i]);
  }
  return bss;
}
void drawBss(Bss bss) {
  for (int i = 0; i < nOfBodies; i++) {
    drawCircle(bss[i].pos, 10, clrs[i], global.drawScale);
  }
}
void printBss(Bss bss) {
  for (int i = 0; i < nOfBodies; i++) {
    std::cout << i << "{\n";
    bss[i].print();
    std::cout << "};\n";
  }
}
int main() {
  InitWindow(global.winSize.x, global.winSize.y, "meow");
  //SetTargetFPS(10);
  camera.position = global.camera_startPos;

  camera.target = global.camera_startTarget;
  camera.up = global.camera_up;
  camera.fovy = global.camera_startFovy;
  camera.projection = CAMERA_PERSPECTIVE;

  Image img = GenImageColor(32, 32, WHITE);
  Texture texture = LoadTextureFromImage(img);
  UnloadImage(img);

  bs_struct a;
  a.v = {4.2, 1, 2};
  bs_struct b;
  b.v = {-4.2, 1, 2};
  Bss bss = {a, b};
  while (!WindowShouldClose()) {
    bss = update(bss);
    printBss(bss);

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

  return 0;
}
