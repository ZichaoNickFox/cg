#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/material.glsl"
#include "renderer/shader/primitive.glsl"

struct BVHNode {
  AABB aabb;
  int sequence_begin;
  int sequence_num;
  int left_node;
  int right_node;
};

uniform int bvh_length;
layout (std430, binding = SSBO_BVH) buffer BVH {
  BVHNode bvh[];
};

struct RayBVHResult {
  bool hitted;
  AABB aabb;
  int primitive_index;
  float dist;
  vec3 normal;
};

Material RayBVHResultMaterial(RayBVHResult result) {
  int material_index = primitive_repo[result.primitive_index].material_index;
  return material_repo[material_index];
}

RayBVHResult RayBVH(Ray ray) {
  RayBVHResult res;
  if (bvh_length == 0) {
    return res;
  }
  int travel[500];
  int top = -1;
  travel[++top] = 0;
  while(bool(top >= 0)) {
    int visit = travel[top--];

    BVHNode visit_node = bvh[visit];
    RayAABBResult ray_aabb_result = RayAABB(ray, visit_node.aabb);
    if (!ray_aabb_result.hitted) {
      continue;
    }
    if (visit_node.left_node == -1 && visit_node.right_node == -1) {
      for (int i = visit_node.sequence_begin; i < visit_node.sequence_begin + visit_node.sequence_num; ++i) {
        BVHNode child_node = ;
        RayTriangleResult ray_triangle_result = RayTriangle(ray, child_node.triangle);
        if (ray_triangle_result.hitted && ray_triangle_result.dist < res.dist) {
          res.hitted = true;
          res.primitive_index = primitive_index;
          res.dist = ray_triangle_result.dist;
          res.aabb = visit_node.aabb;
          res.normal = primitive.normal;
        }
      }
    } else {
      travel[++top] = visit_node.right_node;
      travel[++top] = visit_node.left_node;
    }
  }
  return res;
}