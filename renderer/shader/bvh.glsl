#include "renderer/shader/definition.glsl"
#include "renderer/shader/geometry.glsl"
#include "renderer/shader/material.glsl"
#include "renderer/shader/primitive.glsl"

struct BVH {
  AABB aabb;
  vec4 primitvebegin_primitivenum_leftnode_rightnode;
};

uniform int bvh_num;
layout (std430, binding = SSBO_BVH) buffer BVHBuffer {
  BVH bvh_repo[];
};

int BVHPrimitiveBegin(BVH bvh) {
  return int(bvh.primitvebegin_primitivenum_leftnode_rightnode.x);
}

int BVHPrimitiveNum(BVH bvh) {
  return int(bvh.primitvebegin_primitivenum_leftnode_rightnode.y);
}

int BVHLeftNode(BVH bvh) {
  return int(bvh.primitvebegin_primitivenum_leftnode_rightnode.z);
}

int BVHRightNode(BVH bvh) {
  return int(bvh.primitvebegin_primitivenum_leftnode_rightnode.w);
}

struct RayBVHResult {
  bool hitted;
  AABB aabb;
  int primitive_index;
  float distance;
  vec3 normal;
  vec3 position;
};

RayBVHResult RayBVH(Ray ray) {
  RayBVHResult res;
  res.hitted = false;
  res.distance = FLT_MAX;

  if (bvh_num == 0) {
    return res;
  }
  int travel[500];
  int top = -1;
  travel[++top] = 0;
  while(bool(top >= 0)) {
    int visit = travel[top--];

    BVH visit_node = bvh_repo[visit];
    RayAABBResult ray_aabb_result = RayAABB(ray, visit_node.aabb);
    if (!ray_aabb_result.hitted) {
      continue;
    }
    if (BVHLeftNode(visit_node) == -1 && BVHRightNode(visit_node) == -1) {
      int primitive_begin = BVHPrimitiveBegin(visit_node);
      for (int i = primitive_begin; i < primitive_begin + BVHPrimitiveNum(visit_node); ++i) {
        Primitive primitive = primitive_repo[i];
        RayTriangleResult ray_triangle_result = RayTriangle(ray, PrimitiveTriangle(primitive));
        if (ray_triangle_result.hitted && ray_triangle_result.distance < res.distance) {
          res.hitted = true;
          res.primitive_index = i;
          res.distance = ray_triangle_result.distance;
          res.aabb = visit_node.aabb;
          res.normal = PrimitiveNormal(primitive);
          res.position = ray_triangle_result.position;
        }
      }
    } else {
      travel[++top] = BVHRightNode(visit_node);
      travel[++top] = BVHLeftNode(visit_node);
    }
  }
  return res;
}