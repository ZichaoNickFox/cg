#pragma once

#include <GL/glew.h>

#include "engine/debug.h"

#define CGCHECKGL() int error = glGetError(); CGCHECK(error == 0) << error;

// Common
void glEnable_(GLenum cap);
void glDisable_(GLenum cap);

// Texture
void glGenTextures_(GLsizei n, GLuint *textures);
void glGetTexImage_(GLenum target, GLint level, GLenum format, GLenum type, void * pixels);
void glBindTexture_(GLenum target, GLuint texture);
void glTexImage2D_(GLenum target, GLint level, GLint internalformat, GLsizei width,
                   GLsizei height, GLint border, GLenum format, GLenum type, const void * data);
void glTexSubImage2D_(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width,
                     GLsizei height, GLenum format, GLenum type, const void *pixels);
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

// Shader
GLuint glCreateShader_(GLenum shaderType);
void glShaderSource_(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
void glCompileShader_(GLuint shader);
void glGetShaderiv_(GLuint shader, GLenum pname, GLint *params);
void glGetShaderInfoLog_(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
void glAttachShader_(GLuint program, GLuint shader);
void glLinkProgram_(GLuint program);
void glGetProgramiv_(GLuint program, GLenum pname, GLint *params);
void glGetProgramInfoLog_(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
void glDeleteShader_(GLuint shader);
void glUseProgram_(GLuint program);
GLint glGetUniformLocation_(GLuint program, const GLchar *name);
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