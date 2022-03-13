#pragma once

#include <set>

struct IoInput {
  std::set<std::string> key_input;
  double cursor_x = 0.0;
  double cursor_y = 0.0;
  bool left_button_pressed = false;
  bool right_button_pressed = false;
};