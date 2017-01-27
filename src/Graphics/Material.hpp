#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <memory>

#include <GL/gl3w.h>
#include <GLFW/glfw3.h>

#include <Graphics/UniformGLM.hpp>
#include <Shaders.hpp>
#include <Uniform.hpp>
#include <Texture2D.hpp>
#include <Texture3D.hpp>

#include <Log.hpp>

/**
 * Material
 * Association of a Shader program and a set of Uniforms.
 * @see Program
 * @see Uniform
**/
class Material
{
public:
	//	Constructors
	Material() =default;
	Material(const Program& P);
	Material(Material&&) =default;
	
	/**
	 * Copy constructor
	**/
	Material(const Material&);
	
	/**
	 * Default destructor
	**/
	~Material() =default;
	
	Material& operator=(const Material&);
	Material& operator=(Material&&) =default;

	//	Getters/Setters
	const Program& getShadingProgram() const;
	void setShadingProgram(const Program& P);
	
	const Program* getShadingProgramPtr() const;
	void setShadingProgramPtr(const Program* P);
	
	////////////////////////////////////////////////////////////////
	// Special cases for textures
	
	void setUniform(const std::string& name, const Texture3D& value);
	void setUniform(const std::string& name, const Texture2D& value);
	void setUniform(const std::string& name, const CubeMap& value);
	void setUniform(const std::string& name, const Texture& value);
	
	////////////////////////////////////////////////////////////////
	// Generic Uniform setting
	
	template<typename T>
	inline T getUniform(const std::string&);
	
	template<typename T>
	inline Uniform<T>* searchUniform(const std::string&);
	
	template<typename T>
	inline void setUniform(const std::string& name, const T& value);
	
	inline void setSubroutine(ShaderType stage, const std::string& uniformName, const std::string& functionName);
	
	inline void use() const;
	
	inline void useNone() const;
	
	void bind() const;
	
	void unbind() const;
	
	void updateLocations();
	
	inline const std::vector<std::unique_ptr<GenericUniform>>& getUniforms() const { return _uniforms; }

private:
	const Program*	_shadingProgram = nullptr;
	
	std::vector<std::unique_ptr<GenericUniform>>	_uniforms;
	GLuint 											_textureCount = 0;
	
	class SubroutineState
	{
	public:
		ShaderType							shadertype;
		std::map<std::string, std::string>	activeSubroutines;
		std::vector<GLuint>					activeIndices;

		inline void use() const
		{
			glUniformSubroutinesuiv(to_underlying(shadertype), activeIndices.size(), activeIndices.data());
		}

		void update(const Program& p);
	};
	
	std::map<ShaderType, SubroutineState>	_subroutines;
	
	GLint getLocation(const std::string& name) const;
};

#include "Material.inl"
