#include "engine/shader/ray_tracing/uniform.glsl"
#include "engine/shader/geometry.glsl"

layout (std430, binding = bvn_nodes_binding) buffer BVHNodes { BVHNode bvh_nodes[]; };
layout (std430, binding = triangles_binding) buffer Triangles { Triangle triangles[]; };

struct BVHNode {
  AABB aabb;
  int primitive_start_index;
  int primitive_num; 
  int left;
  int right;
};

struct RayBVHNodeResult {
  bool hitted;
  vec3 pos;
  vec3 normal;
  int triangle_index;
};

Result RayBVHNode(Ray ray) {
  Result res;
  res.hitted = false;

  Node root = bvh_nodes[0]; 
  Node stack[50];
  int top_index = -1;

  top_index++;
  stack[top_index] = root;

  while (top_index >= 0) {
    Node traveling = stack[top_index];
    top_index--;

    if (traveling.left == -1 && traveling.right == -1) {
      // leaf
      for (int i = traveling.primitive_from; i < traveling.primitive_num; ++i) {
        Triangle triangle = triangles[i];
        RayTriangleResult ray_triangle_result = RayTriangle(ray, triangle);
        if (ray_triangle_result.hitted) {
          res.hitted = true;
          res.pos = ray_triangle_result.pos;
          res.normal = ray_triangle_result.normal;
          res.triangle_index = i;
          break;
        }
      }
    } else {
      // internal
      if (RayAABB(ray, traveling.aabb)) {
        if (traveling.left != -1) {
          top_index++;
          stack[top_index] = traveling.left;
        }
        if (traveling.right != -1) {
          top_index++;
          stack[top_index] = traveling.right;
        }
      }
    }
  }
  return res;
}