#include "engine/mesh.h"

#include "GL/glew.h"

namespace engine {

void Mesh::SetPositions(const std::vector<glm::vec3> &positions) {
  positions_ = positions;
}

void Mesh::SetNormals(const std::vector<glm::vec3> &normals) {
  normals_ = normals;
}

void Mesh::SetUvs(const std::vector<glm::vec2> &uvs)
{
  uvs_ = uvs;
}

void Mesh::SetIndices(const std::vector<GLuint> &indices) {
  indices_ = indices;
}

void Mesh::Setup() {
  // calc step
  int byteStep = 0;
  int vertexStep = 0;
  if (positions_.size() > 0)
  {
    byteStep += sizeof(glm::vec3);
    vertexStep += 3;
  }
  if (normals_.size() > 0)
  {
    byteStep += sizeof(glm::vec3);
    vertexStep += 3;
  }
  if (uvs_.size() > 0)
  {
    byteStep += sizeof(glm::vec2);
    vertexStep += 2;
  }
  // fill vs
  int allSize = positions_.size() * vertexStep;
  float *vs = new float[allSize];
  for (int i = 0; i < (int)positions_.size(); ++i)
  {
    int index = 0;
    if (positions_.size() > 0)
    {
      vs[i * vertexStep + index++] = positions_[i].x;
      vs[i * vertexStep + index++] = positions_[i].y;
      vs[i * vertexStep + index++] = positions_[i].z;
    }
    if (normals_.size() > 0)
    {
      vs[i * vertexStep + index++] = normals_[i].x;
      vs[i * vertexStep + index++] = normals_[i].y;
      vs[i * vertexStep + index++] = normals_[i].z;
    }
    if (uvs_.size() > 0)
    {
      vs[i * vertexStep + index++] = uvs_[i].x;
      vs[i * vertexStep + index++] = uvs_[i].y;
    }
  }

  bool use_ebo = indices_.size() > 0;

  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);

  glBindVertexArray(vao_);

  // vbo data
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, allSize * sizeof(float), vs, GL_STATIC_DRAW);
  delete[](vs);

  // ebo data
  if (use_ebo)
  {
    glGenBuffers(1, &ebo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(float), &indices_[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  // vertex data
  int attribIndex = 0;
  int offset = 0;
  if (positions_.size() > 0)
  {
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, byteStep, reinterpret_cast<void*>(offset));
    glEnableVertexAttribArray(attribIndex);
    offset += sizeof(glm::vec3);
    attribIndex++;
  }
  if (normals_.size() > 0)
  {
    glVertexAttribPointer(attribIndex, 3, GL_FLOAT, GL_FALSE, byteStep, reinterpret_cast<void*>(offset));
    glEnableVertexAttribArray(attribIndex);
    offset += sizeof(glm::vec3);
    attribIndex++;
  }
  if (uvs_.size() > 0)
  {
    glVertexAttribPointer(attribIndex, 2, GL_FLOAT, GL_FALSE, byteStep, reinterpret_cast<void*>(offset));
    glEnableVertexAttribArray(attribIndex);
    offset += sizeof(glm::vec2);
    attribIndex++;
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

void Mesh::Submit() {
  bool use_ebo = indices_.size() > 0;
  glBindVertexArray(vao_);
  if (use_ebo)
  {
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, &indices_[0]);
  }
  else
  {
    glDrawArrays(GL_TRIANGLES, 0, positions_.size());
  }
  glBindVertexArray(0);
}
}