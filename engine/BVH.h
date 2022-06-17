#pragma once

#include "engine/geometry.h"
#include "engine/SSBO.h"

#include <vector>

namespace engine {

class BVH {
  struct Node {
    AABB aabb;
    int primitive_start_index;
    int primitive_num = 0;
    int left = -1;
    int right = -1;
    int mesh_instance_id;
  };
 public:
  struct Option {
    int leaf_primitive_size;
    int sah_bucket_num = 64;
  };
  struct Primitive {
    AABB aabb;
    int mesh_instance_id;
    const AABB& GetAABB() const { return aabb; }
  };
  void Build(std::vector<Primitive>* primitives, const Option& option);
  SSBO AsSSBO(int binding_point) const;
  std::vector<AABB> GetAABBs() const;

 private:
  int NewNode(std::vector<Primitive>* aabbs, int begin, int end, const AABB& union_aabb);
  void PartitionNode(std::vector<Primitive>* aabbs, int begin, int end, int node_id);

  struct SAHBucket {
    AABB aabb;
    std::vector<int> primitive_indices;
  };
  int Partition_SAH(std::vector<Primitive>* primitives, int begin, int end, int node_id, int* left_id, int* right_id);
  float SAH_Cost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const;
  float SAH_Cost(const SAHBucket& left, const SAHBucket& right, int node_id) const;
  std::vector<SAHBucket> DivideBuckets(std::vector<Primitive>* primitives, int begin, int end, int node_id);
  int GetMinCostBucket(const std::vector<SAHBucket>& buckets, int node_id);
  int PartitionBuckets(std::vector<SAHBucket>* buckets, int min_cost_bucket, std::vector<Primitive>* primitives,
                       int node_id);
  void ResetPrimitivesByParitition(const std::vector<SAHBucket>& buckets, std::vector<Primitive>* primitives,
                                   int begin, int end, int mid, AABB* left_aabb, AABB* right_aabb);

  void GetAABBs(int node_id, std::vector<AABB>* aabbs) const;

  std::vector<Node> nodes_;
  Option option_;
};

}