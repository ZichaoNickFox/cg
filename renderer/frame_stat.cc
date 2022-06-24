#include "renderer/frame_stat.h"

#include <glm/glm.hpp>
#include "imgui.h"

#include "renderer/debug.h"

namespace renderer {
FrameStat::FrameStat() {
  frame_intervals_.set_capacity(acc_frame_num_);
}

void FrameStat::OnFrame(int last_frame_interval_ms) {
  last_frame_interval_ms_ = last_frame_interval_ms;
  frame_num_++;
  frame_intervals_.push_back(last_frame_interval_ms);
}

void FrameStat::Gui() const {
  ImGui::Text("last frame interval : ");
  ImGui::SameLine();
  ImGui::Text("%d", last_frame_interval());
  ImGui::Text("last FPS : ");
  ImGui::SameLine();
  ImGui::Text("%d", last_fps());
  ImGui::Text("%d average frame interval : %d", acc_frame_num_, avg_frame_interval());
  ImGui::SameLine();
  ImGui::Text("%d", avg_frame_interval());
  ImGui::Text("%d average FPS : %d", avg_frame_num(), avg_fps());
  ImGui::SameLine();
  ImGui::Text("%d", avg_fps());
}

int FrameStat::avg_frame_interval() const {
  int size = frame_intervals_.size();
  int sum = 0;
  for (int i = 0; i < frame_intervals_.size(); ++i) {
    sum += frame_intervals_[i];
  }
  return sum / (size ? size : 1);
}
}
