#pragma once

#include <set>
#include <string>
#include "glm/glm.hpp"

class Io {
 public:
  void SetScreenSize(const glm::ivec2& screen_size) { screen_size_ = screen_size; }
  glm::ivec2 screen_size() const { return screen_size_; }

  void FeedKeyInput(const std::string& key);
  bool HadKeyInput(const std::string& key) const { return key_input_.find(key) != key_input_.end(); }
  void ClearKeyInput() { key_input_.clear(); }

  void FeedCursorPos(double x, double y);
  void FeedButtonInput(bool left, bool right);
  void SetGuiCapturedMouse(bool captured) { gui_captured_cursor_ = captured; }
  double GetCursorDeltaX() const { return cursor_pos_x_ - last_cursor_pos_x_; }
  double GetCursorDeltaY() const { return cursor_pos_y_ - last_cursor_pos_y_; }
  double cursor_screen_pos_x() const { return cursor_pos_x_; } 
  double cursor_screen_pos_y() const { return screen_size_.y - cursor_pos_y_; }
  double normalized_cursor_screen_pos_x() const { return cursor_screen_pos_x() / screen_size_.x; }
  double normalized_cursor_screen_pos_y() const { return cursor_screen_pos_y() / screen_size_.y; }
  bool left_button_pressed() const { return left_button_pressed_; }
  bool right_button_pressed() const { return right_button_pressed_; }
  bool gui_captured_cursor() const { return gui_captured_cursor_; }

 private:
  std::set<std::string> key_input_;
  double cursor_pos_x_ = 0;
  double cursor_pos_y_ = 0;
  double last_cursor_pos_x_ = 0;
  double last_cursor_pos_y_ = 0;
  bool left_button_pressed_ = false;
  bool right_button_pressed_ = false;
  bool gui_captured_cursor_ = false;
  glm::ivec2 screen_size_;
};