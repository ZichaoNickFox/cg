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

  void FeedCursorPos(const glm::vec2& cursor_pos);
  void FeedButtonInput(bool left, bool right);
  void SetGuiCapturedMouse(bool captured) { gui_captured_cursor_ = captured; }
  glm::vec2 GetCursorDelta() const { return cursor_pos_ - last_cursor_pos_; }
  glm::vec2 GetCursorWindowPos() const { return {cursor_pos_.x, screen_size_.y - cursor_pos_.y}; };
  glm::vec2 GetCursorPosSS() const { return GetCursorWindowPos() / glm::vec2(screen_size_); }
  bool left_button_pressed() const { return left_button_pressed_; }
  bool right_button_pressed() const { return right_button_pressed_; }
  bool gui_captured_cursor() const { return gui_captured_cursor_; }

 private:
  std::set<std::string> key_input_;
  glm::vec2 cursor_pos_;
  glm::vec2 last_cursor_pos_;
  bool left_button_pressed_ = false;
  bool right_button_pressed_ = false;
  bool gui_captured_cursor_ = false;
  glm::ivec2 screen_size_;
};