#pragma once

#include <set>
#include <string>

namespace engine {
class Io {
 public:
  void FeedCursorPos(double x, double y);
  void FeedKeyInput(const std::string& key);
  void FeedButtonInput(bool left, bool right);
  void SetGuiCapturedMouse(bool captured) { gui_captured_mouse_ = captured; }

  bool HadKeyInput(const std::string& key) const { return key_input_.find(key) != key_input_.end(); }
  double GetCursorDeltaX() const { return cursor_pos_x_ - last_cursor_pos_x_; }
  double GetCursorDeltaY() const { return cursor_pos_y_ - last_cursor_pos_y_; }
  bool left_button_pressed() const { return left_button_pressed_; }
  bool right_button_pressed() const { return right_button_pressed_; }
  bool gui_captured_mouse() const { return gui_captured_mouse_; }

  void ClearKeyInput() { key_input_.clear(); }

 private:
  std::set<std::string> key_input_;
  double cursor_pos_x_ = 0;
  double cursor_pos_y_ = 0;
  double last_cursor_pos_x_ = 0;
  double last_cursor_pos_y_ = 0;
  bool left_button_pressed_ = false;
  bool right_button_pressed_ = false;
  bool gui_captured_mouse_ = false;
};
}