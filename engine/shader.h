#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "GL/glew.h"

namespace engine {
class Shader
{
public:
  Shader() {}
  Shader(const std::string& vs, const std::string& fs);
  Shader& operator=(const Shader& other);

  void Use();
  void SetBool(const std::string &name, bool value) const;
  void SetInt(const std::string &name, int value) const;
  void SetFloat(const std::string &name, float value) const;
  GLuint id() const { return id_; }

private:
  void CheckCompileErrors(unsigned int shader, const std::string& type);

 private:
  GLuint id_;
};
}