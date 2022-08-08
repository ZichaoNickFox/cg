#pragma once

#include "base/debug.h"

namespace cg {

// Ping means who has the "ball"
template<typename PlayerType>
struct PingPong {
  void Init(PlayerType* player1, PlayerType* player2);
  void SetPing(PlayerType* player) { ping_ = player; }
  PlayerType* ping();
  PlayerType* pong();
  void Swap() { ping_ = ping_ == player1_ ? player2_ : player1_; }

  PlayerType* player1_ = nullptr;
  PlayerType* player2_ = nullptr;
  PlayerType* ping_ = nullptr;
};

template<typename PlayerType>
PlayerType* PingPong<PlayerType>::ping() {
  CGCHECK(ping_) << "Init First";
  return ping_;
}

template<typename PlayerType>
PlayerType* PingPong<PlayerType>::pong() {
  CGCHECK(ping_) << "Init First";
  return ping_ == player1_ ? player2_ : player1_;
}

template<typename PlayerType>
void PingPong<PlayerType>::Init(PlayerType* player1, PlayerType* player2) {
  player1_ = player1;
  player2_ = player2;
  ping_ = player1;
}

} // namespace cg