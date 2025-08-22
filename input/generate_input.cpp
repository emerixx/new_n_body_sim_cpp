#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

void generate_body_setup(){
  json j;
  j["body0"] = {{"pos", {2, -1, 0}}, {"v", {-0.11,1,0}}, {"m", 1}};
  j["body1"] = {{"pos", {-2, -1, 0}}, {"v", {0.2,1,0}}, {"m", 1}};
  j["body2"] = {{"pos", {0, 2, 0}}, {"v", {0,-2,0}}, {"m", 1}};
  std::ofstream o("body_setup.json");
  o <<  j.dump(4) << std::endl;
  o.close();
}
void generate_settings(){
  json j;
  //computation_settings START
  j["computation"]["output_to_file_per_computation_pwr"] = 4;
  j["computation"]["constant_time_step_pwr"] = -6;
  j["computation"]["steps_to_compute_pwr"] = 5;
  j["computation"]["number_of_bodies"] = 3; //TODO: detect based on num of body files
  //computation_settings END 
  //--------------------------------
  //visualization_settings START 
  j["visualization"]["circle_segments"] = 10;
  j["visualization"]["base_grid_spacing"] = 1;
  j["visualization"]["grid_lines"] = 25;
  j["visualization"]["win_size_x"] = 1000;
  j["visualization"]["win_size_y"] = 1000;
  j["visualization"]["zoom_base"] = 10;
  j["visualization"]["circle_radius"] = 3;
  j["visualization"]["trajectories"] = 0;
  j["visualization"]["block_size"] = 1000;
  j["visualization"]["fps"] = 60;
  //visualization_settings END 
  std::ofstream o("settings.json");
  o <<  j.dump(4) << std::endl;
  o.close();

}

int main() {
generate_settings();

  return 0;
}
