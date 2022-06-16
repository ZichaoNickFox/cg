#include "engine/BVH.h"

#include "engine/math.h"
#include "engine/util.h"

namespace engine {

void BVH::Build(const std::vector<AABB>& aabbs, const Option& option) {
  option_ = option;

  int root = NewNode(aabbs, 0, aabbs.size(), UnionAABB(aabbs, 0, aabbs.size()));
  PartitionNode(aabbs, 0, aabbs.size(), root);
}

void BVH::PartitionNode(const std::vector<AABB>& aabbs, int begin, int end, int node_id) {
  if (end - begin <= option_.leaf_primitive_size) {
    return;
  }
  CGCHECK(nodes_[node_id].left == -1 && nodes_[node_id].right == -1);
  int left, right;
  int mid = Partition_SAH(aabbs, begin, end, node_id, &left, &right);
  PartitionNode(aabbs, begin, mid, left);
  PartitionNode(aabbs, mid, end, right);
}

int BVH::NewNode(const std::vector<AABB>& aabbs, int begin, int end, const AABB& union_aabb) {
  int res = nodes_.size();
  Node node;
  node.aabb = union_aabb;
  node.primitive_start_index = begin;
  node.primitive_num = end - begin;
  nodes_.push_back(node);
  return res;
}

int BVH::Partition_SAH(const std::vector<AABB>& aabbs, int begin, int end, int node_id, int* left_id, int* right_id) {
  Node* node = &nodes_[node_id];
  AABB::Axis max_length_axis = node->aabb.GetMaxLengthAxis();
  float max_length = node->aabb.GetLengthByAxis(max_length_axis);
  float min_point = node->aabb.GetMinimumByAxis(max_length_axis);
  float max_point = node->aabb.GetMaximumByAxis(max_length_axis);
  std::vector<SAHBucket> buckets(option_.sah_bucket_num);
  for (int i = begin; i < end; ++i) {
    const AABB& aabb = aabbs.at(i);
    float point = aabb.GetCenterByAxis(max_length_axis);
    int bucket = util::Clamp(int((point - min_point) / (max_point - min_point) * option_.sah_bucket_num),
                             0, option_.sah_bucket_num - 1);
    buckets[bucket].primitive_num++;
    buckets[bucket].aabb.Union(aabb);
  }
  std::vector<SAHBucket> prefix_sum = buckets;
  std::vector<SAHBucket> suffix_sum = buckets;
  for (int i = 1; i < option_.sah_bucket_num; ++i) {
    prefix_sum[i].aabb.Union(prefix_sum[i - 1].aabb);
  }
  for (int i = option_.sah_bucket_num - 2; i >= 0; --i) {
    suffix_sum[i].aabb.Union(suffix_sum[i + 1].aabb);
  }
  float min_cost = std::numeric_limits<float>::max();
  int min_cost_mid;
  for (int mid = 1; mid < option_.sah_bucket_num; ++mid) {
    float cost = SAH_Cost(prefix_sum[mid - 1], suffix_sum[mid], node_id);
    if (min_cost > cost) {
      min_cost = cost;
      min_cost_mid = mid;
    }
    min_cost = std::min(cost, min_cost);
  }
  *left_id = NewNode(aabbs, begin, min_cost_mid, prefix_sum[min_cost_mid - 1].aabb);
  *right_id = NewNode(aabbs, min_cost_mid, end, suffix_sum[min_cost_mid].aabb);
  return min_cost_mid;
}

float BVH::SAH_Cost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const {
  return a.SurfaceArea() / c.SurfaceArea() * na + b.SurfaceArea() / c.SurfaceArea() * nb;
}

float BVH::SAH_Cost(const SAHBucket& left, const SAHBucket& right, int node_id) const {
  return SAH_Cost(left.aabb, right.aabb, nodes_[node_id].aabb, left.primitive_num, right.primitive_num);
}

SSBO BVH::AsSSBO(int binding_point) const {
  SSBO res;
  res.Init(binding_point, util::VectorSizeInByte(nodes_), nodes_.data());
  return res;
}

std::vector<AABB> BVH::GetAABBs() const {
  std::vector<AABB> res; 
  GetAABBs(0, &res);
  return res;
}

void BVH::GetAABBs(int node_id, std::vector<AABB>* aabbs) const {
  if (node_id == -1) {
    return;
  }
  aabbs->push_back(nodes_[node_id].aabb);
  GetAABBs(nodes_[node_id].left, aabbs);
  GetAABBs(nodes_[node_id].right, aabbs);
}

} // namespace engine