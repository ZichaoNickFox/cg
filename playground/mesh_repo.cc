#include "playground/mesh_repo.h"

#include <algorithm>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <glog/logging.h>
#include <math.h>
#include <set>
#include "SOIL2/SOIL2.h"
#include "SOIL2/stb_image.h"

#include "engine/debug.h"
#include "playground/util.h"

class CubeMesh : public engine::Mesh {
 public:
  CubeMesh() {
    std::vector<glm::vec3> positions{
      // back
      glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f),
      glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
      // front
      glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
      glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
      // left 
      glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
      glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),
      // right
      glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
      glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
      // bottom
      glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
      glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
      // top
      glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
      glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f)
    };
    SetPositions(positions);
    std::vector<glm::vec2> texcoords{
      // back
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
      glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
      // front
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
      glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
      // left 
      glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f),
      glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
      // right
      glm::vec2(1.0f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec2(0.0f, 0.0f),
      glm::vec2(0.0f, 0.0f), glm::vec2(0.0f, 1.0f), glm::vec2(1.0f, 1.0f),
      // bottom
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
      glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
      // top
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
      glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
    };
    SetTexcoords(texcoords);
    std::vector<glm::vec3> normals{
      // back
      glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
      glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 0.0f, -1.0f),
      // front
      glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
      glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f),
      // left 
      glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
      glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
      // right
      glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
      glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
      // bottom
      glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
      glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
      // top
      glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)
    };
    SetNormals(normals);
  }
};

class PlaneMesh : public engine::Mesh {
 public:
  PlaneMesh() {
    std::vector<glm::vec3> positions{
      glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f),
      glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, -1.0f),
    };
    SetPositions(positions);
    std::vector<glm::vec3> normals{
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
      glm::vec3(0.0f, 1.0f, 0.0f),
    };
    SetNormals(normals);
    std::vector<glm::vec2> texcoords{
      glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f),
      glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f),
    };
    SetTexcoords(texcoords);
  }
};

class SphereMesh : public engine::Mesh {
 public:
  SphereMesh(int refine = 4) {
    // https://baike.baidu.com/item/%E6%AD%A3%E4%BA%8C%E5%8D%81%E9%9D%A2%E4%BD%93/1921640?fr=aladdin
    // p0 - p11
    // http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
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

    std::vector<glm::vec3> triangles{
      p0, p11, p5, p0, p5, p1, p0, p1, p7, p0, p7, p10, p0, p10, p11,
      p1, p5, p9, p5, p11, p4, p11, p10, p2, p10, p7, p6, p7, p1, p8,
      p3, p9, p4, p3, p4, p2, p3, p2, p6, p3, p6, p8, p3, p8, p9,
      p4, p9, p5, p2, p4, p11, p6, p2, p10, p8, p6, p7, p9, p8, p1,
    };

    while(refine--) {
      std::vector<glm::vec3> refined_triangles; 
      for (int i = 0; i < triangles.size(); i += 3) {
        glm::vec3 a = triangles[i];
        glm::vec3 b = triangles[i + 1];
        glm::vec3 c = triangles[i + 2];
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
      triangles = refined_triangles;
    }
    SetPositions(triangles);
    SetNormals(triangles);

    std::vector<glm::vec2> texcoords(triangles.size());
    std::transform(triangles.begin(), triangles.end(), texcoords.begin(),
        [](const glm::vec3& position){
          float theta = std::atan2(position.z, position.x + 0.1);
          float phi = std::atan(asin(position.y));
          float clamp_theta = (theta + M_PI) / (M_PI * 2);
          float clamp_phi = (phi + 1.0) / 2.0;
          return glm::vec2(clamp_theta, clamp_phi);
        });
    // Some triangle's texcoords cross full texture in the junction of uv. This means:
    // v1 texcoord : (0.95, 0.8)
    // v2 texcoord : (0.9, 0.7)
    // v3 texcoord : (0.1, 0.75)
    // So we must turn 0.1 to [0.9, 0.95] to protect sample full texture in this triangle
    const float threshold = 0.5;
    for (int i = 0; i < texcoords.size(); i += 3) {
      glm::vec2* textcoord1 = &texcoords[i];
      glm::vec2* textcoord2 = &texcoords[i + 1];
      glm::vec2* textcoord3 = &texcoords[i + 2];
      float texcoord_interval12 = std::abs(textcoord1->x - textcoord2->x);
      float texcoord_interval13 = std::abs(textcoord1->x - textcoord3->x);
      float texcoord_interval23 = std::abs(textcoord2->x - textcoord3->x);
      if (texcoord_interval12 > threshold && texcoord_interval13 > threshold) {
         textcoord1->x = (textcoord2->x + textcoord3->x) / 2.0f;
      } else if (texcoord_interval12 > threshold && texcoord_interval23 > threshold) {
         textcoord2->x = (textcoord1->x + textcoord3->x) / 2.0f;
      } else if (texcoord_interval13 > threshold && texcoord_interval23 > threshold) {
         textcoord3->x = (textcoord1->x + textcoord2->x) / 2.0f;
      }
    }
    SetTexcoords(texcoords);
  }
};

void MeshRepo::Init(const Config& config) {
  CGLOG(ERROR) << "Init Mesh : cube";
  meshes_["cube"] = std::make_shared<CubeMesh>();
  meshes_["cube"]->Setup();
  CGLOG(ERROR) << "Init Mesh : Plane";
  meshes_["plane"] = std::make_shared<PlaneMesh>();
  meshes_["plane"]->Setup();
  CGLOG(ERROR) << "Init Mesh : Sphere";
  meshes_["sphere"] = std::make_shared<SphereMesh>();
  meshes_["sphere"]->Setup();
}

std::shared_ptr<engine::Mesh> MeshRepo::GetOrLoadMesh(const std::string& name) {
  CGCHECK(meshes_.count(name) > 0) << " Cannot find in mesh repo : " << name;
  return meshes_[name];
}