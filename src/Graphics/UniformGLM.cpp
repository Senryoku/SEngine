#include <UniformBinding.hpp>

#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

void setUniform(GLint program, GLuint location, const glm::vec2& value)
{
	glProgramUniform2fv(program, location, 1, glm::value_ptr(value));
}

void setUniform(GLint program, GLuint location, const glm::vec3& value)
{
	glProgramUniform3fv(program, location, 1, glm::value_ptr(value));
}

void setUniform(GLint program, GLuint location, const glm::vec4& value)
{
	glProgramUniform4fv(program, location, 1, glm::value_ptr(value));
}

void setUniform(GLint program, GLuint location, const glm::mat2& value)
{
	glProgramUniformMatrix2fv(program, location, 1, false, glm::value_ptr(value));
}

void setUniform(GLint program, GLuint location, const glm::mat3& value)
{
	glProgramUniformMatrix3fv(program, location, 1, false, glm::value_ptr(value));
}

void setUniform(GLint program, GLuint location, const glm::mat4& value)
{
	glProgramUniformMatrix4fv(program, location, 1, false, glm::value_ptr(value));
}

///////////////////////////////////////////////////////////////////
// Pointers

void setUniform(GLint program, GLuint location, const glm::vec2* value)
{
	glProgramUniform2fv(program, location, 1, glm::value_ptr(*value));
}

void setUniform(GLint program, GLuint location, const glm::vec3* value)
{
	glProgramUniform3fv(program, location, 1, glm::value_ptr(*value));
}

void setUniform(GLint program, GLuint location, const glm::vec4* value)
{
	glProgramUniform4fv(program, location, 1, glm::value_ptr(*value));
}

void setUniform(GLint program, GLuint location, const glm::mat2* value)
{
	glProgramUniformMatrix2fv(program, location, 1, false, glm::value_ptr(*value));
}

void setUniform(GLint program, GLuint location, const glm::mat3* value)
{
	glProgramUniformMatrix3fv(program, location, 1, false, glm::value_ptr(*value));
}

void setUniform(GLint program, GLuint location, const glm::mat4* value)
{
	glProgramUniformMatrix4fv(program, location, 1, false, glm::value_ptr(*value));
}

///////////////////////////////////////////////////////////////////
// Non-Program stuff

void setUniform(GLuint location, const glm::vec2& value)
{
	glUniform2fv(location, 1, glm::value_ptr(value));
}

void setUniform(GLuint location, const glm::vec3& value)
{
	glUniform3fv(location, 1, glm::value_ptr(value));
}

void setUniform(GLuint location, const glm::vec4& value)
{
	glUniform4fv(location, 1, glm::value_ptr(value));
}

void setUniform(GLuint location, const glm::mat2& value)
{
	glUniformMatrix2fv(location, 1, false, glm::value_ptr(value));
}

void setUniform(GLuint location, const glm::mat3& value)
{
	glUniformMatrix3fv(location, 1, false, glm::value_ptr(value));
}

void setUniform(GLuint location, const glm::mat4& value)
{
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(value));
}

///////////////////////////////////////////////////////////////////
// Pointers 

void setUniform(GLuint location, const glm::vec2* value)
{
	glUniform2fv(location, 1, glm::value_ptr(*value));
}

void setUniform(GLuint location, const glm::vec3* value)
{
	glUniform3fv(location, 1, glm::value_ptr(*value));
}

void setUniform(GLuint location, const glm::vec4* value)
{
	glUniform4fv(location, 1, glm::value_ptr(*value));
}

void setUniform(GLuint location, const glm::mat2* value)
{
	glUniformMatrix2fv(location, 1, false, glm::value_ptr(*value));
}

void setUniform(GLuint location, const glm::mat3* value)
{
	glUniformMatrix3fv(location, 1, false, glm::value_ptr(*value));
}

void setUniform(GLuint location, const glm::mat4* value)
{
	glUniformMatrix4fv(location, 1, false, glm::value_ptr(*value));
}
