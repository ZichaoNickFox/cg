#pragma once

#include "renderer/definition.h"
#include "renderer/geometry.h"
#include "renderer/primitive.h"
#include "renderer/ssbo.h"

#include <vector>

namespace renderer {

class BVH {
  struct Node {
    AABB aabb;
    int primitives_begin;
    int primitives_num = 0;
    int left_node = -1;
    int right_node = -1;

    bool operator==(const Node& other) const {
      return true;
    }
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
  BVH() : ssbo_(SSBO_BVH) {}
  void Build(const PrimitiveRepo& primitives, const Option& option);
  void UpdateSSBO();
  std::vector<AABB> GetAABBs(int filter_level = -1) const;
  int num() const { return nodes_.size(); }
  std::vector<int> GetPrimitiveSequence() const { return primitive_sequence_; };

 private:
  int NewNode(int begin, int end, const AABB& union_aabb);
  void PartitionNode(const PrimitiveRepo& primitives, int begin, int end, int node_id);
  void GetAABBs(int node_id, int cur_level, int filter_level, std::vector<AABB>* aabbs) const;

  // Partition by SAH begin
  struct SAHBucket {
    AABB aabb;
    std::vector<int> sequence;
  };
  int PartitionBySAH(const PrimitiveRepo& primitives, int begin, int end, int node_id, int* left_node, int* right_node);
  float SAHCost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const;
  float SAHCost(const SAHBucket& left, const SAHBucket& right, int node_id) const;
  std::vector<SAHBucket> DivideBuckets(const PrimitiveRepo& primitives, int begin, int end, int node_id);
  int GetMinCostBucket(const std::vector<SAHBucket>& buckets, int node_id);
  int PartitionBuckets(const PrimitiveRepo& primitives, std::vector<SAHBucket>* buckets, int min_cost_bucket,
                       int node_id);
  void SortSequenceByParitition(const PrimitiveRepo& primitives, const std::vector<SAHBucket>& buckets,
                                int begin, int end, int mid, AABB* left_aabb, AABB* right_aabb);
  // Partition by SAH end

  // Partition by Pos begin
  int PartitionByPos(const PrimitiveRepo& primitives, int begin, int end, int node_id, int* left_node, int* right_node);
  // Partition by Pos end

  // Partition by Pos begin
  int PartitionByNum(const PrimitiveRepo& primitives, int begin, int end, int node_id, int* left_node, int* right_node);
  // Partition by Pos end

  struct NodeGPU {
    NodeGPU() = default;
    NodeGPU(const BVH::Node& node);
    AABBGPU aabb_gpu;
    glm::vec4 primitivebegin_primitivenum_leftnode_rightnode;
  };

  std::vector<Node> nodes_;
  std::vector<Node> dirty_nodes_;
  std::vector<int> primitive_sequence_;
  Option option_;
  SSBO ssbo_;

  friend struct RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const PrimitiveRepo& primitives);
};

struct RayBVHResult {
  bool hitted = false;
  AABB aabb;
  int primitive_index;
  float distance = std::numeric_limits<float>::max();
};
RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const PrimitiveRepo& primitives);
  
}