#include "engine/io.h"

namespace engine {
void Io::FeedCursorPos(double x, double y) {
  last_cursor_pos_x_ = cursor_pos_x_;
  last_cursor_pos_y_ = cursor_pos_y_;
  cursor_pos_x_ = x;
  cursor_pos_y_ = y;
}

void Io::FeedKeyInput(const std::string& key) {
  key_input_.insert(key);
}

void Io::FeedButtonInput(bool left, bool right) {
  left_button_pressed_ = left;
  right_button_pressed_ = right;
}
}