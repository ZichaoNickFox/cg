#include "renderer/io.h"

void Io::FeedCursorPos(const glm::vec2& cursor_pos) {
  last_cursor_pos_ = cursor_pos_;
  cursor_pos_ = cursor_pos;
}

void Io::FeedKeyInput(const std::string& key) {
  key_input_.insert(key);
}

void Io::FeedButtonInput(bool left, bool right) {
  left_button_pressed_ = left;
  right_button_pressed_ = right;
}