#pragma once

#include <boost/circular_buffer.hpp>
#include <queue>

namespace engine {
class FrameStat {
 public:
  FrameStat();
  void OnFrame(int last_frame_interval_ms_);
  int last_frame_interval() const { return last_frame_interval_ms_; }
  int last_fps() const { return 1e3 / (last_frame_interval_ms_ ? last_frame_interval_ms_ : 1); }
  int avg_frame_interval() const;
  int avg_fps() const { return 1e3 / (avg_frame_interval() ? avg_frame_interval() : 1); }
  int avg_frame_num() const { return acc_frame_num_; }
  void Gui() const;

 private:
  int last_frame_interval_ms_ = 0;
  int frame_num_ = 0;

  static constexpr int acc_frame_num_ = 60;
  boost::circular_buffer<int> frame_intervals_;
};
}