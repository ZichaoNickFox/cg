#include "engine/primitive/sphere_mesh.h"

#include "glm/gtx/string_cast.hpp"
#include <map>
#include <math.h>

#include "engine/debug.h"
#include "engine/primitive/primitive_util.h"

SphereMesh::SphereMesh(int refine, const std::string& name) {
  SetName(name);

  // https://baike.baidu.com/item/%E6%AD%A3%E4%BA%8C%E5%8D%81%E9%9D%A2%E4%BD%93/1921640?fr=aladdin
  // p0 - p11
  // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
  std::vector<glm::vec3> triangles;
  FillPositionNormal(refine, &triangles);
  SetPositions(triangles);
  SetNormals(triangles);

  std::vector<glm::vec2> texcoords(triangles.size());
  FillTexcoord(triangles, &texcoords);
  SetTexcoords(texcoords);

  std::vector<glm::vec3> tangents(triangles.size());
  std::vector<glm::vec3> bitangents(triangles.size());
  FillTangentBitangent(triangles, texcoords, &tangents, &bitangents);
  SetTangent(tangents);
  SetBitangent(bitangents);

  Setup();
}

void SphereMesh::FillPositionNormal(int refine, std::vector<glm::vec3>* triangles) {
  float m = std::sqrt(50.0 + 10.0 * std::sqrt(5.0)) / 10.0;
  float n = std::sqrt(50.0 - 10.0 * std::sqrt(5.0)) / 10.0;
  glm::vec3 p9(m, -n, 0.0);
  glm::vec3 p8(m, n, 0.0);
  glm::vec3 p11(-m, -n, 0.0);
  glm::vec3 p10(-m, n, 0.0);
  glm::vec3 p5(0.0, -m, n);
  glm::vec3 p4(0.0, -m, -n);
  glm::vec3 p7(0.0, m, n);
  glm::vec3 p6(0.0, m, -n);
  glm::vec3 p0(-n, 0.0, m);
  glm::vec3 p1(n, 0.0, m);
  glm::vec3 p2(-n, 0.0, -m);
  glm::vec3 p3(n, 0.0, -m);

  *triangles = std::vector<glm::vec3>{
    p0, p11, p5, p0, p5, p1, p0, p1, p7, p0, p7, p10, p0, p10, p11,
    p1, p5, p9, p5, p11, p4, p11, p10, p2, p10, p7, p6, p7, p1, p8,
    p3, p9, p4, p3, p4, p2, p3, p2, p6, p3, p6, p8, p3, p8, p9,
    p4, p9, p5, p2, p4, p11, p6, p2, p10, p8, p6, p7, p9, p8, p1,
  };

  while(refine--) {
    std::vector<glm::vec3> refined_triangles; 
    for (int i = 0; i < triangles->size(); i += 3) {
      glm::vec3 a = triangles->at(i + 0);
      glm::vec3 b = triangles->at(i + 1);
      glm::vec3 c = triangles->at(i + 2);
      glm::vec3 mab = glm::normalize((a + b) / 2.0f);
      glm::vec3 mbc = glm::normalize((b + c) / 2.0f);
      glm::vec3 mac = glm::normalize((a + c) / 2.0f);
      refined_triangles.push_back(a);
      refined_triangles.push_back(mab);
      refined_triangles.push_back(mac);

      refined_triangles.push_back(b);
      refined_triangles.push_back(mab);
      refined_triangles.push_back(mbc);

      refined_triangles.push_back(c);
      refined_triangles.push_back(mbc);
      refined_triangles.push_back(mac);

      refined_triangles.push_back(mab);
      refined_triangles.push_back(mbc);
      refined_triangles.push_back(mac);
    }
    *triangles = refined_triangles;
  }
}

namespace {
//! kTexcoordXThreshold is the factor judging two tecoords cross 01 gap.
// Sphere comes from icosahedron. The pole circle has 5 triangles, max texcoord is 1 / 5 = 0.2
// So When distance of two texcoords > 0.2, Says two texcoords cross 0/1 gap
static constexpr float kTexcoordXThreshold = 1 / 5.0 + 0.1;
static constexpr float pi = 3.1415926f;

// [0, pi * 2)
float CalcTexcoordx(const glm::vec3& position) {
  float phi = std::atan2(position.z, position.x);
  phi = phi < 0 ? phi + 2 * pi : phi;
  return phi / (2 * pi);
}
float CalcTexcoordy(const glm::vec3& position) {
  float theta = asin(position.y);
  theta = theta / (pi / 2.0);
  theta = (theta + 1.0) / 2.0;
  return theta;
}
float CalcAverageOfTexcoords(float texcoordx0, float texcoordx1) {
  // two conditions: 
  // (0.4, 0.5) -> 0.4 + 0.5 / 2 = 0.45
  // (0.1, 0.9) -> (0.1 + 0.9 + 1) / 2 = 1.0
  if (std::abs(texcoordx0 - texcoordx1) > kTexcoordXThreshold) {
    return (texcoordx0 + texcoordx1 + 1) / 2;
  } else {
    return (texcoordx0 + texcoordx1) / 2;
  }
}
}
void SphereMesh::FillTexcoord(const std::vector<glm::vec3>& triangles, std::vector<glm::vec2>* texcoords) {
  // 
  //! Step 1 : texcoordx's pole condition
  // In pole condition, pole's texcoord would be 0, sincce atan2(z=0, x=0) = 0
  // So first replace 0 with other 2 texcoord.xs' average
  //! Step 2 : texcoordx's gap condition
  // Texcoord like (0.9, 0.1) cross 0/1 gap, this condition need handle : 0.1 should be 1.1

  // Step 1 Begin
  for (int triangle_index = 0; triangle_index < triangles.size(); triangle_index += 3) {
    std::map<int, float> texcoordxs;
    int pole_index = -1;
    for (int i = 0; i < 3; ++i) {
      if (std::abs(std::abs(triangles[triangle_index + i].y) - 1) < std::numeric_limits<float>::epsilon()) {
        pole_index = i;
      }
    }
    if (pole_index != -1) {
      float texcoordx_other_index0 = (pole_index + 1) % 3;
      float texcoordx_other_index1 = (pole_index + 2) % 3;
      float texcoordx_other0 = CalcTexcoordx(triangles[triangle_index + texcoordx_other_index0]);
      float texcoordx_other1 = CalcTexcoordx(triangles[triangle_index + texcoordx_other_index1]);
      texcoordxs[texcoordx_other_index0] = texcoordx_other0;
      texcoordxs[texcoordx_other_index1] = texcoordx_other1;
      texcoordxs[pole_index] = CalcAverageOfTexcoords(texcoordx_other0, texcoordx_other1);
    } else {
      texcoordxs[0] = CalcTexcoordx(triangles[triangle_index + 0]);
      texcoordxs[1] = CalcTexcoordx(triangles[triangle_index + 1]);
      texcoordxs[2] = CalcTexcoordx(triangles[triangle_index + 2]);
    }
    // Step 1 End

    // Step 2 Begin
    float* texcoordx0 = &texcoordxs[0];
    float* texcoordx1 = &texcoordxs[1];
    float* texcoordx2 = &texcoordxs[2];
    // Condition 2 : Gap condition and others;
    if (*texcoordx0 > *texcoordx1) {
      std::swap(texcoordx0, texcoordx1);
    }
    if (*texcoordx1 > *texcoordx2) {
      std::swap(texcoordx1, texcoordx2);
    }
    if (*texcoordx0 > *texcoordx1) {
      std::swap(texcoordx0, texcoordx1);
    }
    CGCHECK(*texcoordx0 <= *texcoordx1 && *texcoordx1 <= *texcoordx2 && *texcoordx0 >= 0 && *texcoordx1 >=0
        && *texcoordx2 >=0) << *texcoordx0 << " " << *texcoordx1 << " " << *texcoordx2 << " ";
    if (*texcoordx2 - *texcoordx1 > kTexcoordXThreshold) {
      *texcoordx1 += 1.0;
      *texcoordx0 += 1.0;
    }
    if (*texcoordx1 - *texcoordx0 > kTexcoordXThreshold) {
      *texcoordx0 += 1.0;
    }
    CGCHECK(std::abs(*texcoordx2 - *texcoordx1) < kTexcoordXThreshold) << " " << std::abs(*texcoordx2 - *texcoordx1);
    CGCHECK(std::abs(*texcoordx1 - *texcoordx0) < kTexcoordXThreshold) << " " << std::abs(*texcoordx1 - *texcoordx0);
    CGCHECK(std::abs(*texcoordx0 - *texcoordx2) < kTexcoordXThreshold) << " " << std::abs(*texcoordx0 - *texcoordx2);
    texcoords->at(triangle_index + 0).x = texcoordxs[0];
    texcoords->at(triangle_index + 1).x = texcoordxs[1];
    texcoords->at(triangle_index + 2).x = texcoordxs[2];
    // Step 2 End

    for (int i = 0; i < 3; ++i) {
      float theta = CalcTexcoordy(triangles[triangle_index + i]);
      texcoords->at(triangle_index + i) = glm::vec2(texcoordxs[i], theta);
    }
  }
}

void SphereMesh::FillTangentBitangent(const std::vector<glm::vec3>& triangles, const std::vector<glm::vec2>& texcoords,
                                      std::vector<glm::vec3>* tangents, std::vector<glm::vec3>* bitangents) {
  CGCHECK(triangles.size() == texcoords.size()) << " triangle size != texcoord size";
  for (int i = 0; i < triangles.size(); i += 3) {
    glm::vec3 tangent;
    glm::vec3 bitangent;
    primitive_util::CalcTangentBitangent(triangles[i + 0], triangles[i + 1], triangles[i + 2], 
                                         texcoords[i + 0], texcoords[i + 1], texcoords[i + 2],
                                         &tangent, &bitangent);
    tangents->at(i + 0) = tangent;
    tangents->at(i + 1) = tangent;
    tangents->at(i + 2) = tangent;
    bitangents->at(i + 0) = bitangent;
    bitangents->at(i + 1) = bitangent;
    bitangents->at(i + 2) = bitangent;
  }
}