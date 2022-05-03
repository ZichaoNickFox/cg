#include "engine/gl.h"

void glEnable_(GLenum cap) {
  glEnable(cap);
  CGCHECKGL();
}

void glDisable_(GLenum cap) {
  glDisable(cap);
  CGCHECKGL();
}

void glGenTextures_(GLsizei n, GLuint *textures) {
  glGenTextures(n, textures);
  CGCHECKGL();
}

void glGetTexImage_(GLenum target, GLint level, GLenum format, GLenum type, void * pixels) {
  glGetTexImage(target, level, format, type, pixels);
  CGCHECKGL();
}

void glTexImage2D_(GLenum target, GLint level, GLint internalformat, GLsizei width,
                   GLsizei height, GLint border, GLenum format, GLenum type, const void * data) {
  glTexImage2D(target, level, internalformat, width, height, border, format, type, data);
  CGCHECKGL();
}

void glTexSubImage2D_(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                      GLsizei height, GLenum format, GLenum type, const void *pixels) {
  glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);
  CGCHECKGL();
}

void glTexParameteri_(GLenum target, GLenum pname, GLint param) {
  glTexParameteri(target, pname, param);
  CGCHECKGL();
}

void glBindTexture_(GLenum target, GLuint texture) {
  glBindTexture(target, texture);
  CGCHECKGL();
}

void glGetTexLevelParameteriv_(GLenum target, GLint level, GLenum pname, GLint *params) {
  glGetTexLevelParameteriv(target, level, pname, params);
  CGCHECKGL();
}

void glTexStorage2D_(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height) {
  glTexStorage2D(target, levels, internalformat, width, height);
  CGCHECKGL();
}

void glDeleteTextures_(GLsizei n, const GLuint *textures) {
  glDeleteTextures(n, textures);
  CGCHECKGL();
}

void glGenFramebuffers_(GLsizei n, GLuint *ids) {
  glGenFramebuffers(n, ids);
  CGCHECKGL();
}

void glFramebufferTexture2D_(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
  glFramebufferTexture2D(target, attachment, textarget, texture, level);
  CGCHECKGL();
}

void glBindFramebuffer_(GLenum target, GLuint framebuffer) {
  glBindFramebuffer(target, framebuffer);
  CGCHECKGL();
}

GLenum glCheckFramebufferStatus_(GLenum target) {
  GLenum res = glCheckFramebufferStatus(target);
  CGCHECKGL();
  return res;
}

void glDrawBuffers_(GLsizei n, const GLenum *bufs) {
  glDrawBuffers(n, bufs);
  CGCHECKGL();
}

void glClear_(GLbitfield mask) {
  glClear(mask);
  CGCHECKGL();
}

void glClearColor_(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
  glClearColor(red, green, blue, alpha);
  CGCHECKGL();
}

void glClearBufferiv_(GLenum buffer, GLint drawbuffer, const GLint * value) {
  glClearBufferiv(buffer, drawbuffer, value);
  CGCHECKGL();
}

void glClearBufferuiv_(GLenum buffer, GLint drawbuffer, const GLuint * value) {
  glClearBufferuiv(buffer, drawbuffer, value);
  CGCHECKGL();
}

void glClearBufferfv_(GLenum buffer, GLint drawbuffer, const GLfloat * value) {
  glClearBufferfv(buffer, drawbuffer, value);
  CGCHECKGL();
}

void glClearBufferfi_(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
  glClearBufferfi(buffer, drawbuffer, depth, stencil);
  CGCHECKGL();
}

void glDeleteFramebuffers_(GLsizei n, const GLuint * framebuffers) {
  glDeleteFramebuffers(n, framebuffers);
  CGCHECKGL();
}

GLuint glCreateShader_(GLenum shaderType) {
  GLuint res = glCreateShader(shaderType);
  CGCHECKGL();
  return res;
}

void glShaderSource_(GLuint shader, GLsizei count, const GLchar **string, const GLint *length) {
  glShaderSource(shader, count, string, length);
  CGCHECKGL();
}

void glCompileShader_(GLuint shader) {
  glCompileShader(shader);
  CGCHECKGL();
}

void glGetShaderiv_(GLuint shader, GLenum pname, GLint *params) {
  glGetShaderiv(shader, pname, params);
  CGCHECKGL();
}

void glGetShaderInfoLog_(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {
  glGetShaderInfoLog(shader, maxLength, length, infoLog);
  CGCHECKGL();
}

void glAttachShader_(GLuint program, GLuint shader) {
  glAttachShader(program, shader);
  CGCHECKGL();
}

void glLinkProgram_(	GLuint program) {
  glLinkProgram(program);
  CGCHECKGL();
}

void glGetProgramiv_(GLuint program, GLenum pname, GLint *params) {
  glGetProgramiv(program, pname, params);
  CGCHECKGL();
}

void glGetProgramInfoLog_(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) {
  glGetProgramInfoLog(program, maxLength, length, infoLog);
  CGCHECKGL();
}

void glDeleteShader_(GLuint shader) {
  glDeleteShader(shader);
  CGCHECKGL();
}

void glUseProgram_(GLuint program) {
  glUseProgram(program);
  CGCHECKGL();
}

GLint glGetUniformLocation_(GLuint program, const GLchar *name) {
  GLuint res = glGetUniformLocation(program, name);
  CGCHECKGL();
  return res;
}

void glUniform1f_(GLint location, GLfloat v0) {
  glUniform1f(location, v0);
  CGCHECKGL();
}

void glUniform2f_(GLint location, GLfloat v0, GLfloat v1) {
  glUniform2f(location, v0, v1);
  CGCHECKGL();
}

void glUniform3f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2) {
  glUniform3f(location, v0, v1, v2);
  CGCHECKGL();
}

void glUniform4f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
  glUniform4f(location, v0, v1, v2, v3);
  CGCHECKGL();
}

void glUniform1i_(GLint location, GLint v0) {
  glUniform1i(location, v0);
  CGCHECKGL();
}

void glUniform2i_(GLint location, GLint v0, GLint v1) {
  glUniform2i(location, v0, v1);
  CGCHECKGL();
}

void glUniform3i_(GLint location, GLint v0, GLint v1, GLint v2) {
  glUniform3i(location, v0, v1, v2);
  CGCHECKGL();
}

void glUniform4i_(GLint location, GLint v0, GLint v1, GLint v2, GLint v3) {
  glUniform4i(location, v0, v1, v2, v3);
  CGCHECKGL();
}

void glUniform1ui_(GLint location, GLuint v0) {
  glUniform1ui(location, v0);
  CGCHECKGL();
}

void glUniform2ui_(GLint location, GLuint v0, GLuint v1) {
  glUniform2ui(location, v0, v1);
  CGCHECKGL();
}

void glUniform3ui_(GLint location, GLuint v0, GLuint v1, GLuint v2) {
  glUniform3ui(location, v0, v1, v2);
  CGCHECKGL();
}

void glUniform4ui_(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3) {
  glUniform4ui(location, v0, v1, v2, v3);
  CGCHECKGL();
}

void glUniform1fv_(GLint location, GLsizei count, const GLfloat *value) {
  glUniform1fv(location, count, value);
  CGCHECKGL();
}

void glUniform2fv_(GLint location, GLsizei count, const GLfloat *value) {
  glUniform2fv(location, count, value);
  CGCHECKGL();
}

void glUniform3fv_(GLint location, GLsizei count, const GLfloat *value) {
  glUniform3fv(location, count, value);
  CGCHECKGL();
}

void glUniform4fv_(GLint location, GLsizei count, const GLfloat *value) {
  glUniform4fv(location, count, value);
  CGCHECKGL();
}

void glUniform1iv_(GLint location, GLsizei count, const GLint *value) {
  glUniform1iv(location, count, value);
  CGCHECKGL();
}

void glUniform2iv_(GLint location, GLsizei count, const GLint *value) {
  glUniform2iv(location, count, value);
  CGCHECKGL();
}

void glUniform3iv_(GLint location, GLsizei count, const GLint *value) {
  glUniform3iv(location, count, value);
  CGCHECKGL();
}

void glUniform4iv_(GLint location, GLsizei count, const GLint *value) {
  glUniform4iv(location, count, value);
  CGCHECKGL();
}

void glUniform1uiv_(GLint location, GLsizei count, const GLuint *value) {
  glUniform1uiv(location, count, value);
  CGCHECKGL();
}

void glUniform2uiv_(GLint location, GLsizei count, const GLuint *value) {
  glUniform2uiv(location, count, value);
  CGCHECKGL();
}

void glUniform3uiv_(GLint location, GLsizei count, const GLuint *value) {
  glUniform3uiv(location, count, value);
  CGCHECKGL();
}

void glUniform4uiv_(GLint location, GLsizei count, const GLuint *value) {
  glUniform4uiv(location, count, value);
  CGCHECKGL();
}

void glUniformMatrix2fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix2fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix3fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix3fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix4fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix2x3fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix2x3fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix3x2fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix3x2fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix2x4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix2x4fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix4x2fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix4x2fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix3x4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix3x4fv(location, count, transpose, value);
  CGCHECKGL();
}

void glUniformMatrix4x3fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) {
  glUniformMatrix4x3fv(location, count, transpose, value);
  CGCHECKGL();
}

void glCullFace_(GLenum mode) {
  glCullFace(mode);
  CGCHECKGL();
}

void glFrontFace_(GLenum mode) {
  glFrontFace(mode);
  CGCHECKGL();
}