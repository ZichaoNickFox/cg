#pragma once

#include "engine/geometry.h"
#include "engine/SSBO.h"

#include <vector>

namespace engine {

class BVH {
  struct Node {
    AABB aabb;
    int primitive_start_index;
    int primitive_num;
    int left = -1;
    int right = -1;
  };
  struct SAHBucket {
    int primitive_num;
    AABB aabb;
  };
 public:
  struct Option {
    int leaf_primitive_size;
    int sah_bucket_num = 64;
  };
  void Build(const std::vector<AABB>& aabbs, const Option& option);
  SSBO AsSSBO(int binding_point) const;
  std::vector<AABB> GetAABBs() const;

 private:
  int NewNode(const std::vector<AABB>& aabbs, int begin, int end, const AABB& union_aabb);
  void PartitionNode(const std::vector<AABB>& aabbs, int begin, int end, int node_id);

  int Partition_SAH(const std::vector<AABB>& aabbs, int begin, int end, int node_id, int* left_id, int* right_id);
  float SAH_Cost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const;
  float SAH_Cost(const SAHBucket& left, const SAHBucket& right, int node_id) const;

  void GetAABBs(int node_id, std::vector<AABB>* aabbs) const;

  std::vector<Node> nodes_;
  Option option_;
};

}