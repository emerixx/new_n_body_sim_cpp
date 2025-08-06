#include "global_variables.hpp"
#include "rlgl.h"
#include "vctr.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <raylib.h>

const int nOfBodies = 3;
const double pi = global.pi;
const double G = global.G;
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
using Bss = std::array<Bs, nOfBodies>;

Bs update_bs(Bs bs) {
  double dt = global.constTimeStep * GetFrameTime() * global.speed;
  //std::cout << "dt: " << dt << std::endl;
  bs.pos = bs.pos + bs.v * dt;
  bs.v = bs.v + bs.ds.a * dt;
  return bs;
}

Bss update(Bss bss) {
  double d_mag;
  std::array<double,nOfBodies> f_mag;
  vctr d;
  vctr f;
  for(int i=0; i<nOfBodies; i++){
    f_mag[i]=0;
  }
  for (int i = 0; i < nOfBodies; i++) {
    bss[i].ds.a = {0, 0, 0};
    f=vctr();
    for (int j = 0; j < nOfBodies; j++) {
      if (i == j)
        continue;
      //std::cout << "i|j: " << i << "|" << j << std::endl;
      
      // if i and j werent run b4

      d = (bss[i].pos - bss[j].pos);
      d_mag = d.magnitude();

      //-0.005<d.x<0.005
      if ((d_mag > -0.005 || d_mag < 0.005))
        f_mag[i] = G * bss[j].m / pow(d_mag, 2);

      f = f - f_mag[i] * d.normalised();
      //std::cout << "f_mag: " << f_mag[i] << std::endl;
      //std::cout << "d.norm: " << d.normalised().output_str() << std::endl;
      // {G*bss[j].m/pow(d.x,2),G*bss[j].m/pow(d.y,2),G*bss[j].m/pow(d.z,2)};
    }
    // update acc
    //std::cout << "force: " << f.output_str() << "; i:" << i << std::endl;

    bss[i].ds.a = f / bss[i].m;
    bss[i] = update_bs(bss[i]);
  }
  return bss;
}
void drawBss(Bss bss) {
  for (int i = 0; i < nOfBodies; i++) {
    drawCircle(bss[i].pos, global.circle_radius, clrs[i], global.drawScale);
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
  camera.position = global.camera_startPos;

  camera.target = global.camera_startTarget;
  camera.up = global.camera_up;
  camera.fovy = global.camera_startFovy;
  camera.projection = CAMERA_PERSPECTIVE;

  Image img = GenImageColor(32, 32, WHITE);
  Texture texture = LoadTextureFromImage(img);
  UnloadImage(img);

  Bs a;
  a.pos = {2, 0, 0};
  a.v = {0, -1, 0};
  Bs b;
  b.pos = {-2, 0, 0};
  b.v = {0, 1, 0};
  Bs c;
  c.pos={0, 2, 0};
  c.v={1,0,0};
  Bss bss = {a, b,c};
  while (!WindowShouldClose()) {
    bss = update(bss);
    //printBss(bss);

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
