#pragma once

#include "engine/geometry.h"
#include "engine/primitive.h"
#include "engine/SSBO.h"

#include <vector>

namespace engine {

class BVH {
  struct Node {
    AABB aabb;
    int sequence_begin;
    int sequence_num = 0;
    int left_node = -1;
    int right_node = -1;
    int mesh_instance_id;
  };
 public:
  enum Partition {
    kPos = 0,
    kNum = 1,
    kSAH = 2,
  };
  struct Option {
    int leaf_primitive_size;
    Partition partition_type = Partition::kPos;
    int sah_bucket_num = 64;
  };
  void Build(const Primitives& primitives, const Option& option);
  SSBO AsSSBO(int binding_point) const;
  std::vector<AABB> GetAABBs(int filter_level = -1) const;

 private:
  int NewNode(int begin, int end, const AABB& union_aabb);
  void PartitionNode(const Primitives& primitives, int begin, int end, int node_id);
  void GetAABBs(int node_id, int cur_level, int filter_level, std::vector<AABB>* aabbs) const;

  // Partition by SAH begin
  struct SAHBucket {
    AABB aabb;
    std::vector<int> sequence;
  };
  int PartitionBySAH(const Primitives& primitives, int begin, int end, int node_id, int* left_node, int* right_node);
  float SAHCost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const;
  float SAHCost(const SAHBucket& left, const SAHBucket& right, int node_id) const;
  std::vector<SAHBucket> DivideBuckets(const Primitives& primitives, int begin, int end, int node_id);
  int GetMinCostBucket(const std::vector<SAHBucket>& buckets, int node_id);
  int PartitionBuckets(const Primitives& primitives, std::vector<SAHBucket>* buckets, int min_cost_bucket,
                       int node_id);
  void SortSequenceByParitition(const Primitives& primitives, const std::vector<SAHBucket>& buckets,
                                int begin, int end, int mid, AABB* left_aabb, AABB* right_aabb);
  // Partition by SAH end

  // Partition by Pos begin
  int PartitionByPos(const Primitives& primitives, int begin, int end, int node_id, int* left_node, int* right_node);
  // Partition by Pos end

  // Partition by Pos begin
  int PartitionByNum(const Primitives& primitives, int begin, int end, int node_id, int* left_node, int* right_node);
  // Partition by Pos end

  std::vector<Node> nodes_;
  std::vector<int> sequence_;
  Option option_;

  friend struct RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const Primitives& primitives);
};

struct RayBVHResult {
  bool hitted = false;
  AABB aabb;
  int primitive_index;
  float dist = std::numeric_limits<float>::max();
};
RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const Primitives& primitives);
  
}