#include "engine/BVH.h"

#include "engine/math.h"
#include "engine/util.h"

namespace engine {

void BVH::Build(std::vector<Primitive>* primitives, const Option& option) {
  option_ = option;

  int root = NewNode(primitives, 0, primitives->size(), UnionAABB(*primitives, 0, primitives->size()));
  PartitionNode(primitives, 0, primitives->size(), root);
}

void BVH::PartitionNode(std::vector<Primitive>* primitives, int begin, int end, int node_id) {
  if (end - begin <= option_.leaf_primitive_size) {
    return;
  }
  CGCHECK(nodes_[node_id].left == -1 && nodes_[node_id].right == -1);
  int left, right;
  int mid = Partition_SAH(primitives, begin, end, node_id, &left, &right);
  nodes_[node_id].left = left;
  nodes_[node_id].right = right;
  PartitionNode(primitives, begin, mid, left);
  PartitionNode(primitives, mid, end, right);
}

int BVH::NewNode(std::vector<Primitive>* aabbs, int begin, int end, const AABB& union_aabb) {
  int res = nodes_.size();
  Node node;
  node.aabb = union_aabb;
  node.aabb.CheckValid();
  node.primitive_start_index = begin;
  nodes_.push_back(node);
  return res;
}

int BVH::Partition_SAH(std::vector<Primitive>* primitives, int begin, int end, int node_id, int* left_id, int* right_id) {
  CGCHECK(end - begin >= 2);

  std::vector<SAHBucket> buckets = DivideBuckets(primitives, begin, end, node_id);
  int min_cost_bucket = GetMinCostBucket(buckets, node_id);
  int mid = PartitionBuckets(&buckets, min_cost_bucket, primitives, node_id);
  AABB left_aabb, right_aabb;
  ResetPrimitivesByParitition(buckets, primitives, begin, end, mid, &left_aabb, &right_aabb);

  *left_id = NewNode(primitives, begin, mid, left_aabb);
  *right_id = NewNode(primitives, mid, end, right_aabb);

  CGCHECK(mid >= begin);
  CGCHECK(mid < end);
  return mid;
}

std::vector<BVH::SAHBucket> BVH::DivideBuckets(std::vector<Primitive>* primitives, int begin, int end, int node_id) {
  std::vector<SAHBucket> res(option_.sah_bucket_num);
  Node* node = &nodes_[node_id];
  AABB::Axis max_length_axis = node->aabb.GetMaxLengthAxis();
  float max_length = node->aabb.GetLengthByAxis(max_length_axis);
  float min_point = node->aabb.GetMinimumByAxis(max_length_axis);
  float max_point = node->aabb.GetMaximumByAxis(max_length_axis);
  for (int i = begin; i < end; ++i) {
    const AABB& aabb = primitives->at(i).aabb;
    float point = aabb.GetCenterByAxis(max_length_axis);
    int bucket_index = util::Clamp(int((point - min_point) / (max_point - min_point) * option_.sah_bucket_num),
                                   0, option_.sah_bucket_num - 1);
    res[bucket_index].aabb.Union(aabb);
    res[bucket_index].primitive_indices.push_back(i);
  }
  return res;
}

int BVH::GetMinCostBucket(const std::vector<SAHBucket>& buckets, int node_id) {
  std::vector<SAHBucket> prefix_sum = buckets;
  std::vector<SAHBucket> suffix_sum = buckets;
  for (int i = 1; i < option_.sah_bucket_num; ++i) {
    prefix_sum[i].aabb.Union(prefix_sum[i - 1].aabb);
    prefix_sum[i].primitive_indices.insert(prefix_sum[i].primitive_indices.end(),
                                           prefix_sum[i - 1].primitive_indices.begin(),
                                           prefix_sum[i - 1].primitive_indices.end());
  }
  for (int i = option_.sah_bucket_num - 2; i >= 0; --i) {
    suffix_sum[i].aabb.Union(suffix_sum[i + 1].aabb);
    suffix_sum[i].primitive_indices.insert(suffix_sum[i].primitive_indices.end(),
                                           suffix_sum[i + 1].primitive_indices.begin(),
                                           suffix_sum[i + 1].primitive_indices.end());
  }
  float min_cost = std::numeric_limits<float>::max();
  int min_cost_bucket = -1;
  for (int bucket = 1; bucket < option_.sah_bucket_num; ++bucket) {
    float cost = SAH_Cost(prefix_sum[bucket - 1], suffix_sum[bucket], node_id);
    if (min_cost > cost) {
      min_cost = cost;
      min_cost_bucket = bucket;
    }
  }
  // Cannot split between buckets. All primitive are must be in one backet
  if (min_cost_bucket == -1) {
    for (int i = 0; i < buckets.size(); ++i) {
      const SAHBucket& bucket = buckets[i];
      if (bucket.primitive_indices.size()) {
        CGCHECK(min_cost_bucket == -1) << " Must only one bucket. Leading split between buckets failed";
        min_cost_bucket = i;
      }
    }
  }
  CGCHECK(min_cost_bucket != -1);
  // Attention : min_cost_bucket may be empty.
  CGCHECK(prefix_sum[min_cost_bucket].aabb.CheckNotNull());
  CGCHECK(prefix_sum[min_cost_bucket].primitive_indices.size() > 0);
  CGCHECK(suffix_sum[min_cost_bucket].aabb.CheckNotNull());
  CGCHECK(suffix_sum[min_cost_bucket].primitive_indices.size() > 0);
  return min_cost_bucket;
}

float BVH::SAH_Cost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const {
  return a.SurfaceArea() / c.SurfaceArea() * na + b.SurfaceArea() / c.SurfaceArea() * nb;
}

float BVH::SAH_Cost(const SAHBucket& left, const SAHBucket& right, int node_id) const {
  if (left.primitive_indices.size() == 0 || right.primitive_indices.size() == 0) {
    return std::numeric_limits<float>::max();
  }
  return SAH_Cost(left.aabb, right.aabb, nodes_[node_id].aabb, left.primitive_indices.size(),
                  right.primitive_indices.size());
}

int BVH::PartitionBuckets(std::vector<SAHBucket>* buckets, int min_cost_bucket,
                          std::vector<Primitive>* primitives, int node_id) {
  Node* node = &nodes_[node_id];
  AABB::Axis max_length_axis = node->aabb.GetMaxLengthAxis();

  int mid = -1;
  if (buckets->at(min_cost_bucket).primitive_indices.size() > 0) {
    SAHBucket* bucket = &buckets->at(min_cost_bucket);
    // Attention : min_cost_bucket may be empty.
    auto le_compare = [primitives, max_length_axis] (int left_index, int right_index) {
      float pos_left = primitives->at(left_index).aabb.GetCenterByAxis(max_length_axis);
      float pos_right = primitives->at(right_index).aabb.GetCenterByAxis(max_length_axis);
      return pos_left <= pos_right;
    };
    CGCHECK(bucket->primitive_indices.size() > 0);
    mid = util::QuickSelect(&bucket->primitive_indices,
                            bucket->primitive_indices.size() / 2, le_compare);
  } else {
    bool found = false;
    for (int i = min_cost_bucket; i < buckets->size(); ++i) {
      SAHBucket* bucket = &buckets->at(i);
      if (bucket->primitive_indices.size() > 0) {
        auto iter = std::min_element(bucket->primitive_indices.begin(), bucket->primitive_indices.end(),
            [primitives, max_length_axis] (int left, int right) {
              return primitives->at(left).aabb.GetCenterByAxis(max_length_axis)
                  < primitives->at(right).aabb.GetCenterByAxis(max_length_axis);
            });
        int index = std::distance(bucket->primitive_indices.begin(), iter);
        std::swap(bucket->primitive_indices[0], bucket->primitive_indices[index]);
        mid = bucket->primitive_indices[0];
        found = true;
        break;
      }
    }
    CGCHECK(found);
  }
  CGCHECK(mid != -1) << "Logic Error";
  return mid;
}

void BVH::ResetPrimitivesByParitition(const std::vector<SAHBucket>& buckets, std::vector<Primitive>* primitives,
                                      int begin, int end, int mid, AABB* left_aabb, AABB* right_aabb) {
  std::vector<Primitive> partitioned_primitives(end - begin);
  int partitioned_primitive_index = 0;
  for (int bucket = 0; bucket < option_.sah_bucket_num; ++bucket) {
    for (const int primitive_index : buckets.at(bucket).primitive_indices) {
      partitioned_primitives[partitioned_primitive_index] = primitives->at(primitive_index);
      partitioned_primitives[partitioned_primitive_index].aabb.CheckNotNull();
      if (partitioned_primitive_index < mid) {
        left_aabb->Union(partitioned_primitives[partitioned_primitive_index].aabb);
      } else {
        right_aabb->Union(partitioned_primitives[partitioned_primitive_index].aabb);
      }
      partitioned_primitive_index++;
    }
  }
  CGCHECK(end - begin == partitioned_primitive_index) << end - begin << " " << partitioned_primitive_index;
  CGCHECK(partitioned_primitives.size() == end - begin) << partitioned_primitives.size() << end - begin;
  util::VectorOverride(partitioned_primitives, 0, partitioned_primitives.size(), primitives, begin);
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