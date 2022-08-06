#include "renderer/BVH.h"

#include <stack>

#include "base/math.h"
#include "base/util.h"

namespace renderer {

void BVH::Build(const PrimitiveRepo& primitive_repo, const Option& option) {
  option_ = option;
  primitive_sequence_.resize(primitive_repo.num());
  for (int i = 0; i < primitive_repo.num(); ++i) {
    primitive_sequence_[i] = i;
  }
  int root_id = NewNode(0, primitive_sequence_.size(), UnionAABB(primitive_repo.data(), primitive_sequence_));
  PartitionNode(primitive_repo, 0, primitive_sequence_.size(), root_id);
}

void BVH::PartitionNode(const PrimitiveRepo& primitives, int begin, int end, int node_id) {
  if (end - begin <= option_.leaf_primitive_size) {
    nodes_[node_id].primitives_begin = begin;
    nodes_[node_id].primitives_num = end - begin;
    return;
  }
  CGCHECK(nodes_[node_id].left_node == -1 && nodes_[node_id].right_node == -1)
      << node_id << " " << nodes_[node_id].left_node << " " << nodes_[node_id].right_node;
  int left_node, right_node;
  int mid;
  if (option_.partition_type == Partition::kPos) {
    mid = PartitionByPos(primitives, begin, end, node_id, &left_node, &right_node);
  } else if (option_.partition_type == Partition::kNum) {
    mid = PartitionByNum(primitives, begin, end, node_id, &left_node, &right_node);
  } else if (option_.partition_type == Partition::kSAH) {
    mid = PartitionBySAH(primitives, begin, end, node_id, &left_node, &right_node);
  } else {
    CGKILL("Unsupported Partition type") << option_.partition_type;
  }
  nodes_[node_id].left_node = left_node;
  nodes_[node_id].right_node = right_node;
  PartitionNode(primitives, begin, mid, left_node);
  PartitionNode(primitives, mid, end, right_node);
}

int BVH::NewNode(int begin, int end, const AABB& union_aabb) {
  int res = nodes_.size();
  Node node;
  node.aabb = union_aabb;
#if CGDEBUG
  node.aabb.DebugCheckValid();
#endif
  node.primitives_begin = begin;
  node.primitives_num = end - begin;
  nodes_.push_back(node);
  return res;
}

int BVH::PartitionBySAH(const PrimitiveRepo& primitives, int begin, int end, int node_id, int* left_id, int* right_id) {
  CGCHECK(end - begin >= 2);

  std::vector<SAHBucket> buckets = DivideBuckets(primitives, begin, end, node_id);
  int min_cost_bucket = GetMinCostBucket(buckets, node_id);
  int mid = PartitionBuckets(primitives, &buckets, min_cost_bucket, node_id);
  AABB left_aabb, right_aabb;
  SortSequenceByParitition(primitives, buckets, begin, end, mid, &left_aabb, &right_aabb);

  *left_id = NewNode(begin, mid, left_aabb);
  *right_id = NewNode(mid, end, right_aabb);

  CGCHECK(mid >= begin);
  CGCHECK(mid < end);
  return mid;
}

std::vector<BVH::SAHBucket> BVH::DivideBuckets(const PrimitiveRepo& primitives, int begin, int end, int node_id) {
  std::vector<SAHBucket> res(option_.sah_bucket_num);
  Node* node = &nodes_[node_id];
  AABB::Axis max_length_axis = node->aabb.GetMaxLengthAxis();
  float max_length = node->aabb.GetLengthByAxis(max_length_axis);
  float min_point = node->aabb.GetMinimumByAxis(max_length_axis);
  float max_point = node->aabb.GetMaximumByAxis(max_length_axis);
  for (int i = begin; i < end; ++i) {
    const AABB& aabb = primitives.GetAABB(primitive_sequence_[i]);
    float point = aabb.GetCenterByAxis(max_length_axis);
    int bucket_index = math::Clamp(int((point - min_point) / (max_point - min_point) * option_.sah_bucket_num),
                                   0, option_.sah_bucket_num - 1);
    res[bucket_index].aabb.Union(aabb);
    res[bucket_index].sequence.push_back(i);
  }
  return res;
}

int BVH::GetMinCostBucket(const std::vector<SAHBucket>& buckets, int node_id) {
  std::vector<SAHBucket> prefix_sum = buckets;
  std::vector<SAHBucket> suffix_sum = buckets;
  for (int i = 1; i < option_.sah_bucket_num; ++i) {
    prefix_sum[i].aabb.Union(prefix_sum[i - 1].aabb);
    prefix_sum[i].sequence.insert(prefix_sum[i].sequence.end(),
                                  prefix_sum[i - 1].sequence.begin(),
                                  prefix_sum[i - 1].sequence.end());
  }
  for (int i = option_.sah_bucket_num - 2; i >= 0; --i) {
    suffix_sum[i].aabb.Union(suffix_sum[i + 1].aabb);
    suffix_sum[i].sequence.insert(suffix_sum[i].sequence.end(),
                                  suffix_sum[i + 1].sequence.begin(),
                                  suffix_sum[i + 1].sequence.end());
  }
  float min_cost = std::numeric_limits<float>::max();
  int min_cost_bucket = -1;
  for (int bucket = 1; bucket < option_.sah_bucket_num; ++bucket) {
    float cost = SAHCost(prefix_sum[bucket - 1], suffix_sum[bucket], node_id);
    if (min_cost > cost) {
      min_cost = cost;
      min_cost_bucket = bucket;
    }
  }
  // Cannot split between buckets. All primitive are must be in one backet
  if (min_cost_bucket == -1) {
    for (int i = 0; i < buckets.size(); ++i) {
      const SAHBucket& bucket = buckets[i];
      if (bucket.sequence.size()) {
        CGCHECK(min_cost_bucket == -1) << " Must only one bucket. Leading split between buckets failed";
        min_cost_bucket = i;
      }
    }
  }
  CGCHECK(min_cost_bucket != -1);
  // Attention : min_cost_bucket may be empty.
  CGCHECK(prefix_sum[min_cost_bucket].aabb.DebugCheckNotNull());
  CGCHECK(prefix_sum[min_cost_bucket].sequence.size() > 0);
  CGCHECK(suffix_sum[min_cost_bucket].aabb.DebugCheckNotNull());
  CGCHECK(suffix_sum[min_cost_bucket].sequence.size() > 0);
  return min_cost_bucket;
}

float BVH::SAHCost(const AABB& a, const AABB& b, const AABB& c, int na, int nb) const {
  return a.SurfaceArea() / c.SurfaceArea() * na + b.SurfaceArea() / c.SurfaceArea() * nb;
}

float BVH::SAHCost(const SAHBucket& left_bucket, const SAHBucket& right_bucket, int node_id) const {
  if (left_bucket.sequence.size() == 0 || right_bucket.sequence.size() == 0) {
    return std::numeric_limits<float>::max();
  }
  return SAHCost(left_bucket.aabb, right_bucket.aabb, nodes_[node_id].aabb, left_bucket.sequence.size(),
                  right_bucket.sequence.size());
}

int BVH::PartitionBuckets(const PrimitiveRepo& primitives, std::vector<SAHBucket>* buckets, int min_cost_bucket,
                          int node_id) {
  Node* node = &nodes_[node_id];
  AABB::Axis max_length_axis = node->aabb.GetMaxLengthAxis();

  int mid = -1;
  if (buckets->at(min_cost_bucket).sequence.size() > 0) {
    SAHBucket* bucket = &buckets->at(min_cost_bucket);
    // Attention : min_cost_bucket may be empty.
    auto le_compare = [&primitives, max_length_axis, this] (int left_index, int right_index) {
      float pos_left = primitives.GetAABB(primitive_sequence_[left_index]).GetCenterByAxis(max_length_axis);
      float pos_right = primitives.GetAABB(primitive_sequence_[right_index]).GetCenterByAxis(max_length_axis);
      return pos_left <= pos_right;
    };
    CGCHECK(bucket->sequence.size() > 0);
    mid = math::QuickSelect(&bucket->sequence, 0, bucket->sequence.size(), bucket->sequence.size() / 2, le_compare);
  } else {
    bool found = false;
    for (int i = min_cost_bucket; i < buckets->size(); ++i) {
      SAHBucket* bucket = &buckets->at(i);
      if (bucket->sequence.size() > 0) {
        auto iter = std::min_element(bucket->sequence.begin(), bucket->sequence.end(),
            [&primitives, max_length_axis, this] (int left, int right) {
              return primitives.GetAABB(primitive_sequence_[left]).GetCenterByAxis(max_length_axis)
                  < primitives.GetAABB(primitive_sequence_[right]).GetCenterByAxis(max_length_axis);
            });
        int index = std::distance(bucket->sequence.begin(), iter);
        std::swap(bucket->sequence[0], bucket->sequence[index]);
        mid = bucket->sequence[0];
        found = true;
        break;
      }
    }
    CGCHECK(found);
  }
  CGCHECK(mid != -1) << "Logic Error";
  return mid;
}

void BVH::SortSequenceByParitition(const PrimitiveRepo& primitives, const std::vector<SAHBucket>& buckets,
                                   int begin, int end, int mid, AABB* left_aabb, AABB* right_aabb) {
  std::vector<int> sorted_sequence(end - begin);
  int sorted_sequence_index = 0;
  for (int bucket = 0; bucket < option_.sah_bucket_num; ++bucket) {
    for (const int sequence_index : buckets.at(bucket).sequence) {
      sorted_sequence[sorted_sequence_index] = primitive_sequence_[sequence_index];
#if CGDEBUG
      primitives.GetAABB(sorted_sequence[sorted_sequence_index]).DebugCheckNotNull();
#endif
      if (sorted_sequence_index < mid) {
        left_aabb->Union(primitives.GetAABB(sorted_sequence[sorted_sequence_index]));
      } else {
        right_aabb->Union(primitives.GetAABB(sorted_sequence[sorted_sequence_index]));
      }
      sorted_sequence_index++;
    }
  }
  CGCHECK(end - begin == sorted_sequence_index) << end - begin << " " << sorted_sequence_index;
  CGCHECK(sorted_sequence.size() == end - begin) << sorted_sequence.size() << end - begin;
  util::VectorOverride(&primitive_sequence_, begin, sorted_sequence, 0, sorted_sequence.size());
}

int BVH::PartitionByPos(const PrimitiveRepo& primitives, int begin, int end, int node_id,
                        int* left_node, int* right_node) {
  CGCHECK(node_id < nodes_.size());
  Node* node = &nodes_[node_id];
  AABB::Axis max_length_axis = node->aabb.GetMaxLengthAxis();
  auto le_compare = [&primitives, max_length_axis, this] (int left_value, int right_value) {
    float pos_left = primitives.GetAABB(left_value).GetCenterByAxis(max_length_axis);
    float pos_right = primitives.GetAABB(right_value).GetCenterByAxis(max_length_axis);
    return pos_left <= pos_right;
  };
  int mid = math::QuickSelect(&primitive_sequence_, begin, end, begin + (end - begin) / 2, le_compare);
  AABB left_aabb;
  for (int i = begin; i < mid; ++i) {
    left_aabb.Union(primitives.GetAABB(primitive_sequence_[i]));
  }
  AABB right_aabb;
  for (int i = mid; i < end; ++i) {
    right_aabb.Union(primitives.GetAABB(primitive_sequence_[i]));
  }
  *left_node = NewNode(begin, mid, left_aabb);
  *right_node = NewNode(mid, end, right_aabb);
  return mid;
}

int BVH::PartitionByNum(const PrimitiveRepo& primitives, int begin, int end, int node_id,
                        int* left_node, int* right_node) {
  CGKILL("To Be Implement");
  return 0;
}

BVH::NodeGPU::NodeGPU(const BVH::Node& node) {
  aabb_gpu = AABBGPU(node.aabb);
  primitivebegin_primitivenum_leftnode_rightnode
      = glm::vec4(node.primitives_begin, node.primitives_num, node.left_node, node.right_node);
}

void BVH::UpdateSSBO() {
  bool dirty = !(nodes_ == dirty_nodes_);
  if (dirty) {
    std::vector<NodeGPU> ssbo_data;
    std::transform(nodes_.begin(), nodes_.end(), std::back_inserter(ssbo_data),
                   [] (const Node& node) { return NodeGPU(node); });
    ssbo_.SetData(util::VectorSizeInByte(ssbo_data), ssbo_data.data());
    dirty_nodes_ = nodes_;
  }
}

std::vector<AABB> BVH::GetAABBs(int filter_level) const {
  std::vector<AABB> res; 
  GetAABBs(0, 0, filter_level, &res);
  return res;
}

void BVH::GetAABBs(int node_id, int cur_level, int filter_level, std::vector<AABB>* aabbs) const {
  if (node_id == -1) {
    return;
  }
  if (filter_level == -1 || filter_level == cur_level) {
    aabbs->push_back(nodes_[node_id].aabb);
#if CGDEBUG
    aabbs->back().SetColor(cur_level);
#endif
  }
  GetAABBs(nodes_[node_id].left_node, cur_level + 1, filter_level, aabbs);
  GetAABBs(nodes_[node_id].right_node, cur_level + 1, filter_level, aabbs);
}

RayBVHResult RayBVH(const Ray& ray, const BVH& bvh, const PrimitiveRepo& primitive_repo) {
  RayBVHResult res;
  if (bvh.nodes_.size() == 0) {
    return res;
  }
  std::stack<int> travel;
  travel.push(0);
  while(!travel.empty()) {
    int visit = travel.top();
    travel.pop();

    BVH::Node node = bvh.nodes_[visit];
    RayAABBResult ray_aabb_result = RayAABB(ray, node.aabb);
    if (!ray_aabb_result.hitted) {
      continue;
    }
    if (node.left_node == -1 && node.right_node == -1) {
      CGCHECK(node.primitives_num > 0) << node.primitives_num;
      for (int i = node.primitives_begin; i < node.primitives_begin + node.primitives_num; ++i) {
        CGCHECK(i < bvh.primitive_sequence_.size()) << i << " " << bvh.primitive_sequence_.size();
        int primitive_index = bvh.primitive_sequence_[i];
        CGCHECK(primitive_index < primitive_repo.num()) << primitive_index << " " << primitive_repo.num();
        RayTriangleResult ray_triangle_result = RayTriangle(ray, primitive_repo.GetTriangle(primitive_index));
        if (ray_triangle_result.hitted && ray_triangle_result.distance < res.distance) {
          res.hitted = true;
          res.primitive_index = primitive_index;
          res.distance = ray_triangle_result.distance;
          res.aabb = node.aabb;
        }
      }
    } else {
      travel.push(node.right_node);
      travel.push(node.left_node);
    }
  }
  return res;
}

} // namespace renderer