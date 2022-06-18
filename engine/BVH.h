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
    int left = -1;
    int right = -1;
    int mesh_instance_id;
  };
 public:
  struct Option {
    int leaf_primitive_size;
    int sah_bucket_num = 64;
  };
  void Build(const std::vector<Primitive>& primitives, const Option& option);
  SSBO AsSSBO(int binding_point) const;
  std::vector<AABB> GetAABBs(int filter_level = -1) const;

 private:
  struct Primitives {
    const AABB& GetAABB(int index) const { return primitives->at(index).aabb; }
    const std::vector<Primitive>* primitives = nullptr;
  };
  int NewNode(int begin, int end, const AABB& union_aabb);
  void PartitionNode(const Primitives& primitives, int begin, int end, int node_id);

  struct SAHBucket {
    AABB aabb;
    std::vector<int> sequence;
  };
  int Partition_SAH(const Primitives& primitives, int begin, int end, int node_id, int* left_id, int* right_id);
  float SAH_Cost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const;
  float SAH_Cost(const SAHBucket& left, const SAHBucket& right, int node_id) const;
  std::vector<SAHBucket> DivideBuckets(const Primitives& primitives, int begin, int end, int node_id);
  int GetMinCostBucket(const std::vector<SAHBucket>& buckets, int node_id);
  int PartitionBuckets(const Primitives& primitives, std::vector<SAHBucket>* buckets, int min_cost_bucket,
                       int node_id);
  void SortSequenceByParitition(const Primitives& primitives, const std::vector<SAHBucket>& buckets,
                                int begin, int end, int mid, AABB* left_aabb, AABB* right_aabb);

  void GetAABBs(int node_id, int cur_level, int filter_level, std::vector<AABB>* aabbs) const;

  std::vector<Node> nodes_;
  std::vector<int> sequence_;
  Option option_;

  friend struct RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const std::vector<Primitive>& primitives);
};

struct RayBVHResult {
  bool hitted = false;
  int primitive_index;
  float dist = std::numeric_limits<float>::max();
};
RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const std::vector<Primitive>& primitives);
  
}