#include <Material.hpp>

Material::Material(const Program& P) :
	_shadingProgram(&P)
{
}
	
Material::Material(const Material& m)
{
	_shadingProgram = m._shadingProgram;
	for(const auto& u : m._uniforms)
		_uniforms.push_back(std::unique_ptr<GenericUniform>(u.get()->clone()));
	_textureCount = m._textureCount;
	_subroutines = m._subroutines;
}

Material& Material::operator=(const Material& m)
{
	_shadingProgram = m._shadingProgram;
	for(const auto& u : m._uniforms)
		_uniforms.push_back(std::unique_ptr<GenericUniform>(u.get()->clone()));
	_textureCount = m._textureCount;
	_subroutines = m._subroutines;
	
	return *this;
}

void Material::bind() const
{	
	for(const auto& U : _uniforms)
		U.get()->bind(_shadingProgram->getName());
}

void Material::unbind() const
{	
	for(auto& U : _uniforms)
		U.get()->unbind(_shadingProgram->getName());
}

void Material::updateLocations()
{
	for(auto& U : _uniforms)
	{
		U.get()->setLocation(getLocation(U.get()->getName()));
	}
}
	
GLint Material::getLocation(const std::string& name) const
{
	return _shadingProgram->getUniformLocation(name);
}

void Material::SubroutineState::update(const Program& p)
{
	GLsizei uniformCount;
	glGetProgramStageiv(p.getName(), to_underlying(shadertype),
			GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &uniformCount);
			 
	if(uniformCount == 0)
	{
		Log::error("This program have no active subroutine.");
		return;
	}
	
	activeIndices.resize(uniformCount, 0);
	
	for(auto& s : activeSubroutines)
	{
		GLint uniformLocation = glGetSubroutineUniformLocation(p.getName(), to_underlying(shadertype), s.first.c_str());
		if(uniformLocation < 0)
		{
			Log::error("Uniform ", s.first, " not found.");
		} else {
			GLuint r = p.getSubroutineIndex(shadertype, s.second.c_str());

			if(r == GL_INVALID_INDEX)
			{
				Log::error("Subroutine '", s.second, "' not found.");
			} else {
				activeIndices[uniformLocation] = r;
			}
		}
	}
}
