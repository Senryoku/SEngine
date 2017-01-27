#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

/**
 * GLM extension for SenOGL uniform bindings.
**/

void setUniform(GLint program, GLuint location, const glm::vec2& value);
void setUniform(GLint program, GLuint location, const glm::vec3& value);
void setUniform(GLint program, GLuint location, const glm::vec4& value);
void setUniform(GLint program, GLuint location, const glm::quat& value);
void setUniform(GLint program, GLuint location, const glm::mat2& value);
void setUniform(GLint program, GLuint location, const glm::mat3& value);
void setUniform(GLint program, GLuint location, const glm::mat4& value);

void setUniform(GLint program, GLuint location, const glm::vec2* value);
void setUniform(GLint program, GLuint location, const glm::vec3* value);
void setUniform(GLint program, GLuint location, const glm::vec4* value);
void setUniform(GLint program, GLuint location, const glm::quat* value);
void setUniform(GLint program, GLuint location, const glm::mat2* value);
void setUniform(GLint program, GLuint location, const glm::mat3* value);
void setUniform(GLint program, GLuint location, const glm::mat4* value);

void setUniform(GLuint location, const glm::vec2& value);
void setUniform(GLuint location, const glm::vec3& value);
void setUniform(GLuint location, const glm::vec4& value);
void setUniform(GLuint location, const glm::quat& value);
void setUniform(GLuint location, const glm::mat2& value);
void setUniform(GLuint location, const glm::mat3& value);
void setUniform(GLuint location, const glm::mat4& value);

void setUniform(GLuint location, const glm::vec2* value);
void setUniform(GLuint location, const glm::vec3* value);
void setUniform(GLuint location, const glm::vec4* value);
void setUniform(GLuint location, const glm::quat* value);
void setUniform(GLuint location, const glm::mat2* value);
void setUniform(GLuint location, const glm::mat3* value);
void setUniform(GLuint location, const glm::mat4* value);
