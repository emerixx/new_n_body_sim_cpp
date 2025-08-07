#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

int main() {
  json j;
  j["body0"] = {{"pos", {2, -1, 0}}, {"v", {-0.11,1,0}}, {"m", 1}};
  j["body1"] = {{"pos", {-2, -1, 0}}, {"v", {0.2,1,0}}, {"m", 1}};
  j["body2"] = {{"pos", {0, 2, 0}}, {"v", {0,-2,0}}, {"m", 1}};
  std::ofstream o("body_setup.json");
  o <<  j.dump(4) << std::endl;
  o.close();

  return 0;
}
