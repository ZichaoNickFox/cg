#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"

layout (std430, binding = SSBO_BVH) buffer BVH { BVHNode bvh_nodes[]; };

struct BVHNode {
  AABB aabb;
  int sequence_begin;
  int sequence_num = 0;
  int left_node = -1;
  int right_node = -1;
};

struct RayBVHResult {
  bool hitted = false;
  AABB aabb;
  int primitive_index;
  float dist = FLT_MAX;
  vec3 normal;
};

RayBVHResult RayBVHNode(Ray ray) {
  RayBVHResult res;
  if (bvh.bvh_nodes.length() == 0) {
    return res;
  }
  int travel[500];
  int top = -1;
  travel[++top] = 0;
  while(!top >= 0) {
    int visit = travel[top--];

    BVHNode node = bvh.nodes_[visit];
    RayAABBResult ray_aabb_result = RayAABB(ray, node.aabb);
    if (!ray_aabb_result.hitted) {
      continue;
    }
    if (node.left_node == -1 && node.right_node == -1) {
      for (int i = node.sequence_begin; i < node.sequence_begin + node.sequence_num; ++i) {
        int primitive_index = bvh.sequence_[i];
        RayTriangleResult ray_triangle_result = RayTriangle(ray, primitives[primitive_index].triangle);
        if (ray_triangle_result.hitted && ray_triangle_result.dist < res.dist) {
          res.hitted = true;
          res.primitive_index = primitive_index;
          res.dist = ray_triangle_result.dist;
          res.aabb = node.aabb;
          res.normal = primitives[primitive_index].normal;
        }
      }
    } else {
      travel[++top] = node.right_node;
      travel[++top] = node.left_node;
    }
  }
  return res;
}