#include "engine/gl.h"

GLenum glewInit_() {
  GLenum res = glewInit();
  CGCHECKGL();
  return res;
}

void glEnable_(GLenum cap) {
  glEnable(cap);
  CGCHECKGL();
}

void glDisable_(GLenum cap) {
  glDisable(cap);
  CGCHECKGL();
}

void glViewport_(GLint x, GLint y, GLsizei width, GLsizei height) {
  glViewport(x, y, width, height);
  CGCHECKGL();
}

void glGetBooleanv_(GLenum pname, GLboolean* data) {
  glGetBooleanv(pname, data);
  CGCHECKGL();
}

void glGetDoublev_(GLenum pname, GLdouble* data) {
  glGetDoublev(pname, data);
  CGCHECKGL();
}

void glGetFloatv_(GLenum pname, GLfloat* data) {
  glGetFloatv(pname, data);
  CGCHECKGL();
}

void glGetIntegerv_(GLenum pname, GLint* data) {
  glGetIntegerv(pname, data);
  CGCHECKGL();
}

void glGetInteger64v_(GLenum pname, GLint64* data) {
  glGetInteger64v(pname, data);
  CGCHECKGL();
}

void glGetBooleani_v_(GLenum target, GLuint index, GLboolean* data) {
  glGetBooleani_v(target, index, data);
  CGCHECKGL();
}

void glGetIntegeri_v_(GLenum target, GLuint index, GLint* data) {
  glGetIntegeri_v(target, index, data);
  CGCHECKGL();
}

void glGetFloati_v_(GLenum target, GLuint index, GLfloat* data) {
  glGetFloati_v(target, index, data);
  CGCHECKGL();
}

void glGetDoublei_v_(GLenum target, GLuint index, GLdouble* data) {
  glGetDoublei_v(target, index, data);
  CGCHECKGL();
}

void glGetInteger64i_v_(GLenum target, GLuint index, GLint64* data) {
  glGetInteger64i_v(target, index, data);
  CGCHECKGL();
}

void glGetInternalformativ_(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params) {
  glGetInternalformativ(target, internalformat, pname, bufSize, params);
  CGCHECKGL();
}

void glGenBuffers_(GLsizei n, GLuint* buffers) {
  glGenBuffers(n, buffers);
  CGCHECKGL();
}

void glBindBuffer_(GLenum target, GLuint buffer) {
  glBindBuffer(target, buffer);
  CGCHECKGL();
}

void glBufferData_(GLenum target, GLsizeiptr size, const void * data, GLenum usage) {
  glBufferData(target, size, data, usage);
  CGCHECKGL();
}

void glBufferSubData_(GLenum target, GLintptr offset, GLsizeiptr size, const void * data) {
  glBufferSubData(target, offset, size, data);
  CGCHECKGL();
}

void glGenVertexArrays_(GLsizei n, GLuint *arrays) {
  glGenVertexArrays(n, arrays);
  CGCHECKGL();
}

void glBindVertexArray_(GLuint array) {
  glBindVertexArray(array);
  CGCHECKGL();
}

void glEnableVertexAttribArray_(GLuint index) {
  glEnableVertexAttribArray(index);
  CGCHECKGL();
}

void glVertexAttribPointer_(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) {
  glVertexAttribPointer(index, size, type, normalized, stride, pointer);
  CGCHECKGL();
}

void glDrawArrays_(GLenum mode, GLint first, GLsizei count) {
  glDrawArrays(mode, first, count);
  CGCHECKGL();
}

void glDrawArraysInstanced_(GLenum mode, GLint first, GLsizei count, GLsizei instancecount) {
  glDrawArraysInstanced(mode, first, count, instancecount);
  CGCHECKGL();
}

void glVertexAttribDivisor_(GLuint index, GLuint divisor) {
  glVertexAttribDivisor(index, divisor);
  CGCHECKGL();
}

void glDrawElements_(GLenum mode, GLsizei count, GLenum type, const void* indices) {
  glDrawElements(mode, count, type, indices);
  CGCHECKGL();
}

void glDrawElementsInstanced_(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount) {
  glDrawElementsInstanced(mode, count, type, indices, instancecount);
  CGCHECKGL();
}

void glDeleteVertexArrays_(GLsizei n, const GLuint *arrays) {
  glDeleteVertexArrays(n, arrays);
  CGCHECKGL();
}

void glDeleteBuffers_(GLsizei n, const GLuint * buffers) {
  glDeleteBuffers(n, buffers);
  CGCHECKGL();
}

void glGenTextures_(GLsizei n, GLuint *textures) {
  glGenTextures(n, textures);
  CGCHECKGL();
}

void glGetTexImage_(GLenum target, GLint level, GLenum format, GLenum type, void * pixels) {
  glGetTexImage(target, level, format, type, pixels);
  CGCHECKGL() << "glGetTextureImage";
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

void glTexImage2DMultisample_(GLenum target, GLsizei samples, GLenum internalformat,
                              GLsizei width, GLsizei height, GLboolean fixedsamplelocations) {
  glTexImage2DMultisample(target, samples, internalformat, width, height, fixedsamplelocations);
  CGCHECKGL();
}

void glTexParameteri_(GLenum target, GLenum pname, GLint param) {
  glTexParameteri(target, pname, param);
  CGCHECKGL();
}

void glBindTexture_(GLenum target, GLuint texture) {
  glBindTexture(target, texture);
  CGCHECKGL() << target << " " << texture;
}

void glBindImageTexture_(GLuint unit, GLuint texture, GLint level, GLboolean layered,
                         GLint layer, GLenum access, GLenum format) {
  glBindImageTexture(unit, texture, level, layered, layer, access, format);
  CGCHECKGL();
}

void glActiveTexture_(GLenum texture) {
  glActiveTexture(texture);
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
  CGCHECKGL() << std::hex
              << "\ntarget - " << target << "\nattachment - " << attachment
              << "\ntextarget - "<< textarget << "\ntexture - "<< texture
              << "\nlevel - " << level
              << std::dec;
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

void glClearBufferuiv_(GLenum buffer, GLint drawbuffer, const GLuint* value) {
  glClearBufferuiv(buffer, drawbuffer, value);
  CGCHECKGL();
}

void glClearBufferfv_(GLenum buffer, GLint drawbuffer, const GLfloat* value) {
  glClearBufferfv(buffer, drawbuffer, value);
  CGCHECKGL() << "\nglClearBufferfv" << std::hex
              << "\nbuffer - " << buffer
              << "\ndrawbuffer - " << drawbuffer
              << "\nvalue - " << value[0] << " " << value[1] << " " << value[2] << " " << value[3]
              << std::dec;
}

void glClearBufferfi_(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil) {
  glClearBufferfi(buffer, drawbuffer, depth, stencil);
  CGCHECKGL();
}

void glDeleteFramebuffers_(GLsizei n, const GLuint* framebuffers) {
  glDeleteFramebuffers(n, framebuffers);
  CGCHECKGL();
}

void glBlitFramebuffer_(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                        GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter) {
  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);
  CGCHECKGL();
}

void glDrawBuffer_(GLenum buf) {
  glDrawBuffer(buf);
  CGCHECKGL();
}

void glReadBuffer_(GLenum buf) {
  glReadBuffer(buf);
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

GLuint glCreateProgram_() {
  GLuint res = glCreateProgram();
  CGCHECKGL();
  return res;
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

GLboolean glIsProgram_(GLuint program) {
  bool res = glIsProgram(program);
  CGCHECKGL();
  return res;
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

void glDispatchCompute_(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) {
  glDispatchCompute(num_groups_x, num_groups_y, num_groups_z);
  CGCHECKGL();
}

void glMemoryBarrier_(GLbitfield barriers) {
  glMemoryBarrier(barriers);
  CGCHECKGL();
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
  CGCHECKGL() << "glUniform3fv_ :"
              << " location ~ " << location
              << " count ~ " << count;
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

void glDepthFunc_(GLenum func) {
  glDepthFunc(func);
  CGCHECKGL();
}

void glDepthMask_(GLboolean flag) {
  glDepthMask(flag);
  CGCHECKGL();
}
