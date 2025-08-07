#include "rlgl.h"
#include "vctr.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <string>

const int circle_segments = 10;
const int n_of_bodies = 3;
const int block_size=10000;
const double pi = 3.1415926535897932384626433832795028841971693993751;
const Color clrs[] = {RED, BLUE, GREEN};
const Vector3 winSize = {1000, 1000};
const Vector3 winCenter = {winSize.x / 2, winSize.y / 2};
const int grid_spacing = 20;
const int grid_lines = 25;
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

using pos_arr = std::array<std::array<vctr, n_of_bodies>,block_size>;
pos_arr load_next_block(pos_arr current_pos_arr){

}

int main() {
  InitWindow(winSize.x, winSize.y, "meow");
  camera.position = camera_startPos;

  camera.target = camera_startTarget;
  camera.up = camera_up;
  camera.fovy = camera_startFovy;
  camera.projection = CAMERA_PERSPECTIVE;

  Image img = GenImageColor(32, 32, WHITE);
  Texture texture = LoadTextureFromImage(img);
  UnloadImage(img);
  int x = 0;
  while (!WindowShouldClose()) {

    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(0, 0);

    BeginMode3D(camera);
    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    drawGrid(grid_lines, grid_spacing, WHITE);
    rlPopMatrix();
    //drawBss(bss);
    EndMode3D();
    EndDrawing();
    x++;
  }
  for (int i; i < n_of_bodies; i++)
    fo[i].close();
  return 0;
}
