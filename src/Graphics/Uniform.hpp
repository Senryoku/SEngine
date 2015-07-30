#pragma once

#include <UniformBinding.hpp>

class GenericUniform
{
public:
	GenericUniform() =default;
	GenericUniform(const std::string& N, GLuint L) :
		_name(N),
		_location(L)
	{ }
	virtual ~GenericUniform() =default;
	
	inline const std::string& getName() const { return _name; }
	inline GLuint getLocation() const { return _location; }
	
	inline void setLocation(const GLuint val) { _location = val; }
	
	virtual void bind() =0;
	virtual void bind(GLuint program) =0;
	
	virtual void unbind(GLuint program) {}
	
	/**
	 * Construct a new Uniform with the same attributes as this one.
	 * Used for Material copying.
	**/
	virtual GenericUniform* clone() const =0;

private:
	std::string		_name;
	GLuint				_location;
};

template<typename T>
class Uniform : public GenericUniform
{
public:
	Uniform() =default;
	Uniform(const std::string& N, GLuint L, const T& V) :
		GenericUniform(N, L),
		_value(V)
	{ }
	virtual ~Uniform() =default;
	
	inline const T& getValue() const { return _value; }
	inline T& getRefToValue() { return _value; }
	
	inline void setValue(const T& val) { _value = val; }
	
	virtual inline void bind() override
	{
		setUniform(getLocation(), getValue());
	}
	
	virtual inline void bind(GLuint program) override
	{
		setUniform(program,  getLocation(), getValue());
	}
	
	virtual Uniform<T>* clone() const override
	{
		return new Uniform<T>(getName(), getLocation(), getValue());
	}

protected:
	T					_value;
};

#include <Texture.hpp>

template<>
class Uniform<Texture> : public GenericUniform
{
public:
	Uniform() =default;
	Uniform(const std::string& N, GLuint L, GLuint U, const Texture& V) :
		GenericUniform(N, L),
		_value(&V),
		_textureUnit(U)
	{ }
	virtual ~Uniform() =default;
	
	inline const Texture& getValue() const { return *_value; }
	inline GLuint getTextureUnit() const { return _textureUnit; }
	
	inline void setValue(const Texture& val) { _value = &val; }
	inline void setTextureUnit(GLuint U) { _textureUnit = U; }
	
	virtual inline void bind() override
	{
		_value->bind(_textureUnit);
		setUniform(getLocation(),  static_cast<GLint>(getTextureUnit()));
	}
	
	virtual inline void bind(GLuint program) override
	{
		_value->bind(_textureUnit);
		setUniform(program, getLocation(),  static_cast<GLint>(getTextureUnit()));
	}
	
	virtual inline void unbind(GLuint program) override
	{
		_value->unbind(_textureUnit);
	}
	
	virtual Uniform<Texture>* clone() const override
	{
		return new Uniform<Texture>(getName(), getLocation(), getTextureUnit(), getValue());
	}
	
private:
	const Texture*		_value;
	GLuint 				_textureUnit = 0;
};
