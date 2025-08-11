#include "../global_variables.hpp"
#include "vctr.hpp"
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <raylib.h>
#include <string>

const int circle_segments = 10;
const int n_of_bodies = 2;
const int block_size = 10000;
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

using pos_arr = std::array<std::array<vctr, block_size>, n_of_bodies>;
bool hit_end;
int end_line;
int blocks_read;
vctr line_to_vctr(std::string ln) {
  char commas[2];
  vctr out;
  ln = ln.substr(1, ln.size() - 2);
  std::stringstream ss(ln);
  if (ss >> out.x >> commas[0] >> out.y >> commas[1] >> out.z) {
    if (!(commas[0] == ',' && commas[1] == ',')) {
      std::cerr << "Format error: commas missing or misplaced\n";
    }
  } else {
    std::cerr << "Parsing error\n";
  }
  return out;
}

pos_arr load_next_block(std::ifstream fin[n_of_bodies]) {
  pos_arr out;
  std::string line;
  for (int i = 0; i < n_of_bodies; i++) {
    for (int j = 0; j < block_size; j++) {
      if (std::getline(fin[0], line)) {
        out[i][j] = line_to_vctr(line);
      } else {
        hit_end = true;
        end_line = blocks_read * block_size + j;
        std::cout << "End of file (body" << i << ") at line" << end_line
                  << "\n";
        break;
      }
    }
  }
  blocks_read++;
  std::cout << "loaded next block\n";
  return out;
  /*
  std::stringstream ss(line);
  if (ss >> out_temp.x >> comma1 >> out_temp.y >> comma2 >> out_temp.z) {
    if (comma1 == ',' && comma2 == ',') {
      std::cout << "x = " << out_temp.x << ", y = " << out_temp.y << ", z = " <<
  out_temp.z << std::endl; } else { std::cerr << "Format error: commas missing
  or misplaced\n";
    }
  } else {
    std::cerr << "Parsing error\n";
  }
  std::cout << line << std::endl;
  line = line.substr(1, line.size() - 2);
  std::cout << line << std::endl;*/
  // while (std::getline(fin[0], line)) {
  //}
}

int main() {
  InitWindow(winSize.x, winSize.y, "meow");
  SetTargetFPS(60);
  camera.position = camera_startPos;
  pos_arr current_block;
  camera.target = camera_startTarget;
  camera.up = camera_up;
  camera.fovy = camera_startFovy;
  camera.projection = CAMERA_PERSPECTIVE;

  Image img = GenImageColor(32, 32, WHITE);
  Texture texture = LoadTextureFromImage(img);
  UnloadImage(img);

  std::ifstream fin[n_of_bodies];
  for (int i = 0; i < n_of_bodies; i++) {
    fin[i].open("../" + global.output_dir + "body" + std::to_string(i));
    if (!fin[i]) {
      std::cerr << "Failed to open "
                << "../" + global.output_dir + "body" + std::to_string(i)
                << std::endl;
      return 1;
    }
  }
  int x = 0;
  current_block = load_next_block(fin);
  while (!WindowShouldClose()) {

    BeginDrawing();
    ClearBackground(BLACK);
    DrawFPS(0, 0);

    BeginMode3D(camera);
    rlPushMatrix();
    rlRotatef(90, 1, 0, 0);
    drawGrid(grid_lines, grid_spacing, WHITE);
    rlPopMatrix();

    if (x < block_size || hit_end) {
      // dont read block
      for (int i = 0; i < n_of_bodies; i++) {
        drawCircle(current_block[i][x], 5, clrs[i], 1);
      }
    } else if (!hit_end) {
      x = 0;
      current_block = load_next_block(fin);
    }

    EndMode3D();
    EndDrawing();
    if (!hit_end) {
      x++;
    }
  }
  for (int i; i < n_of_bodies; i++)
    fin[i].close();
  return 0;
}
