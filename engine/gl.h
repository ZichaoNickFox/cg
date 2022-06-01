#pragma once

#include "GL/glew.h"

#include "engine/debug.h"

#define CGCHECKGL() int error = glGetError(); CGCHECK(error == 0) << std::hex << error << std::dec << " "

// glew
GLenum glewInit_();

// Common
void glEnable_(GLenum cap);
void glDisable_(GLenum cap);
void glViewport_(GLint x, GLint y, GLsizei width, GLsizei height);
void glGetBooleanv_(GLenum pname, GLboolean* data);
void glGetDoublev_(GLenum pname, GLdouble* data);
void glGetFloatv_(GLenum pname, GLfloat* data);
void glGetIntegerv_(GLenum pname, GLint* data);
void glGetInteger64v_(GLenum pname, GLint64* data);
void glGetBooleani_v_(GLenum target, GLuint index, GLboolean* data);
void glGetIntegeri_v_(GLenum target, GLuint index, GLint* data);
void glGetFloati_v_(GLenum target, GLuint index, GLfloat* data);
void glGetDoublei_v_(GLenum target, GLuint index, GLdouble* data);
void glGetInteger64i_v_(GLenum target, GLuint index, GLint64* data);
void glGetInternalformativ_(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params);

// Mesh
void glGenBuffers_(GLsizei n, GLuint* buffers);
void glBindBuffer_(GLenum target, GLuint buffer);
void glBufferData_(GLenum target, GLsizeiptr size, const void * data, GLenum usage);
void glBufferSubData_(GLenum target, GLintptr offset, GLsizeiptr size, const void * data);
void glGenVertexArrays_(GLsizei n, GLuint *arrays);
void glBindVertexArray_(GLuint array);
void glEnableVertexAttribArray_(GLuint index);
void glVertexAttribPointer_(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
void glDrawArrays_(GLenum mode, GLint first, GLsizei count);
void glDrawArraysInstanced_(GLenum mode, GLint first, GLsizei count, GLsizei instancecount);
void glVertexAttribDivisor_(GLuint index, GLuint divisor);
void glDrawElements_(GLenum mode, GLsizei count, GLenum type, const void* indices);
void glDrawElementsInstanced_(GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei instancecount);
void glDeleteVertexArrays_(GLsizei n, const GLuint *arrays);
void glDeleteBuffers_(GLsizei n, const GLuint * buffers);

// Texture
void glGenTextures_(GLsizei n, GLuint *textures);
void glGetTexImage_(GLenum target, GLint level, GLenum format, GLenum type, void * pixels);
void glBindTexture_(GLenum target, GLuint texture);
void glBindImageTexture_(GLuint unit, GLuint texture, GLint level, GLboolean layered,
                         GLint layer, GLenum access, GLenum format);
void glActiveTexture_(GLenum texture);
void glTexImage2D_(GLenum target, GLint level, GLint internalformat, GLsizei width,
                   GLsizei height, GLint border, GLenum format, GLenum type, const void * data);
void glTexSubImage2D_(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                     GLsizei height, GLenum format, GLenum type, const void *pixels);
void glTexImage2DMultisample_(GLenum target, GLsizei samples, GLenum internalformat,
                              GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
void glGetTexLevelParameteriv_(GLenum target, GLint level, GLenum pname, GLint *params);
void glTexParameteri_(GLenum target, GLenum pname, GLint param);
void glTexStorage2D_(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
void glDeleteTextures_(GLsizei n, const GLuint *textures);

// Framebuffer
void glGenFramebuffers_(GLsizei n, GLuint *ids);
void glFramebufferTexture2D_(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void glBindFramebuffer_(GLenum target, GLuint framebuffer);
GLenum glCheckFramebufferStatus_(GLenum target);
void glDrawBuffers_(GLsizei n, const GLenum *bufs);
void glClear_(GLbitfield mask);
void glClearColor_(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void glClearBufferiv_(GLenum buffer, GLint drawbuffer, const GLint * value);
void glClearBufferuiv_(GLenum buffer, GLint drawbuffer, const GLuint * value);
void glClearBufferfv_(GLenum buffer, GLint drawbuffer, const GLfloat * value);
void glClearBufferfi_(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil);
void glDeleteFramebuffers_(GLsizei n, const GLuint * framebuffers);
void glBlitFramebuffer_(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                        GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
void glDrawBuffer_(GLenum buf);
void glReadBuffer_(GLenum buf);

// Shader
GLuint glCreateShader_(GLenum shaderType);
void glShaderSource_(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
void glCompileShader_(GLuint shader);
void glGetShaderiv_(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog_(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
void glAttachShader_(GLuint program, GLuint shader);
GLuint glCreateProgram_();
void glLinkProgram_(GLuint program);
void glGetProgramiv_(GLuint program, GLenum pname, GLint *params);
void glGetProgramInfoLog_(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
void glDeleteShader_(GLuint shader);
GLboolean glIsProgram_(GLuint program);
void glUseProgram_(GLuint program);
GLint glGetUniformLocation_(GLuint program, const GLchar *name);
void glDispatchCompute_(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
void glMemoryBarrier_(GLbitfield barriers);
void glUniform1f_(GLint location, GLfloat v0);
void glUniform2f_(GLint location, GLfloat v0, GLfloat v1);
void glUniform3f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
void glUniform4f_(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
void glUniform1i_(GLint location, GLint v0);
void glUniform2i_(GLint location, GLint v0, GLint v1);
void glUniform3i_(GLint location, GLint v0, GLint v1, GLint v2);
void glUniform4i_(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
void glUniform1ui_(GLint location, GLuint v0);
void glUniform2ui_(GLint location, GLuint v0, GLuint v1);
void glUniform3ui_(GLint location, GLuint v0, GLuint v1, GLuint v2);
void glUniform4ui_(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3);
void glUniform1fv_(GLint location, GLsizei count, const GLfloat *value);
void glUniform2fv_(GLint location, GLsizei count, const GLfloat *value);
void glUniform3fv_(GLint location, GLsizei count, const GLfloat *value);
void glUniform4fv_(GLint location, GLsizei count, const GLfloat *value);
void glUniform1iv_(GLint location, GLsizei count, const GLint *value);
void glUniform2iv_(GLint location, GLsizei count, const GLint *value);
void glUniform3iv_(GLint location, GLsizei count, const GLint *value);
void glUniform4iv_(GLint location, GLsizei count, const GLint *value);
void glUniform1uiv_(GLint location, GLsizei count, const GLuint *value);
void glUniform2uiv_(GLint location, GLsizei count, const GLuint *value);
void glUniform3uiv_(GLint location, GLsizei count, const GLuint *value);
void glUniform4uiv_(GLint location, GLsizei count, const GLuint *value);
void glUniformMatrix2fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix3fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix2x3fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix3x2fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix2x4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix4x2fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix3x4fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
void glUniformMatrix4x3fv_(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

// Cull face
void glCullFace_(GLenum mode);
void glFrontFace_(GLenum mode);

// Depth, Stencil
void glDepthFunc_(GLenum func);
void glDepthMask_(GLboolean flag);