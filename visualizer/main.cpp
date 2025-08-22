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

int circle_segments = 10;
const int n_of_bodies = 3;
const int block_size = 1000; // has to be const cuz array
const double pi = 3.1415926535897932384626433832795028841971693993751;
const Color clrs[] = {RED, BLUE, GREEN};
Vector3 winSize;
Vector3 winCenter;
int base_grid_spacing = 1;
int grid_lines = 25;
int fps = 60;
Camera camera;
Image img;
Texture texture;
bool is_stopped = false;
const Vector3 camera_startPos = {0, 0, 250};
const Vector3 camera_startTarget = {0, 0, 0};
const Vector3 camera_up = {0, 1, 0};
const float camera_startFovy = 60;
float circle_radius = 5;
double zoom = 1;
int zoom_base = 10;

void load_settings_from_file() {
  std::ifstream fin("../input/settings.json");
  if (fin) {
    std::cout << "Settings file opened successfully\n";
  } else {
    std::cerr << "Couldnt open settings file, exiting...\n";
    exit(-1);
  }
  nlohmann::json j;
  int pwr;
  fin >> j;
  fin.close();
  circle_radius = j["visualization"]["circle_radius"];
  circle_segments = j["visualization"]["circle_segments"];
  fps = j["visualization"]["fps"];
  grid_lines = j["visualization"]["grid_lines"];
  base_grid_spacing = j["visualization"]["base_grid_spacing"];
  // trajectories = j["visualization"]["trajectories"];
  winSize.x = j["visualization"]["win_size_x"];
  winSize.y = j["visualization"]["win_size_y"];
  zoom_base = j["visualization"]["zoom_base"];
}

void drawGrid(int slices, float spacing, Color color) {

  int halfSlices = slices / 2;

  rlBegin(RL_LINES);
  for (int i = -halfSlices; i <= halfSlices; i++) {

    rlColor3f(color.r / 255.f, color.g / 255.f, color.b / 255.f);

    rlVertex3f((float)i * spacing * zoom, 0.0f,
               (float)-halfSlices * spacing * zoom);
    rlVertex3f((float)i * spacing * zoom, 0.0f,
               (float)halfSlices * spacing * zoom);

    rlVertex3f((float)-halfSlices * spacing * zoom, 0.0f,
               (float)i * spacing * zoom);
    rlVertex3f((float)halfSlices * spacing * zoom, 0.0f,
               (float)i * spacing * zoom);
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
  std::cout << "loading next block, blocks loaded/read: " << blocks_read
            << std::endl;
  for (int i = 0; i < n_of_bodies; i++) {
    for (int j = 0; j < block_size; j++) {
      std::cout << j << std::endl;
      if (std::getline(fin[i], line)) {

        out[i][j] = line_to_vctr(line);
        if (i == 0) {
          // only count blocks read for the 1st body
        }
      } else {
        hit_end = true;
        end_line = blocks_read * block_size + j;
        std::cout << "End of file (body" << i << ") at line" << end_line
                  << "\n";
        // break;
      }
    }
  }
  blocks_read++;

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
  load_settings_from_file();

  winSize = {800, 600};
  winCenter = {winSize.x / 2, winSize.y / 2};

  InitWindow(winSize.x, winSize.y, "meow");
  SetTargetFPS(fps);
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
    drawGrid(grid_lines, base_grid_spacing, WHITE);
    rlPopMatrix();

    if (x < block_size || hit_end) {
      // dont read block
      std::cout << x << "/" << block_size << std::endl;
      for (int i = 0; i < n_of_bodies; i++) {
        drawCircle(current_block[i][x], circle_radius, clrs[i], zoom);
      }
      if (x % 500 == 0) {
        // recalc zoom and centerOffset
        double r = 0;
        // r=sum(pos[j].mag)

        for (int j = 0; j < n_of_bodies; j++) {
          r = r + current_block[j][x].magnitude();
        }
        if(r*zoom_base>zoom*1.5){
          zoom=zoom_base*r;
        }
      }
    } else if (!hit_end) {
      x = 0;
      current_block = load_next_block(fin);
    }

    EndMode3D();
    EndDrawing();
    if (!hit_end || x < end_line - block_size * blocks_read) {
      x++;
    }
  }
  for (int i; i < n_of_bodies; i++)
    fin[i].close();
  return 0;
}
