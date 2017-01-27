#include <serialization.hpp>

#include <glm/gtx/transform.hpp>

glm::mat4 mat4(const nlohmann::json& json)
{
	glm::mat4 r;
	if(json.is_array())
	{
		for(int i = 0; i < 4; ++i)
			for(int j = 0; j < 4; ++j)
				r[i][j] = json[i * 4 + j];
	} else {
		r = glm::translate(glm::mat4(1.0f), vec3(json["position"])) * 
			glm::mat4_cast(quat(json["rotation"])) * 
			glm::scale(glm::mat4(1.0f), vec3(json["scale"]));
	}
	return r;
}

#include <hash.hpp>

void update_material(const nlohmann::json& v, Material& m)
{
	for(const auto& u : v)
	{
		switch(hash(u[1].get<std::string>().c_str()))
		{
			case hash("int"):
				m.setUniform<int>(u[0], u[2]);
				break;
			case hash("float"):
				m.setUniform<float>(u[0], u[2]);
				break;
			case hash("vec3"):
				m.setUniform(u[0], vec3(u[2]));
				break;
			case hash("vec4"):
				m.setUniform(u[0], vec4(u[2]));
				break;
			case hash("quat"):
				m.setUniform(u[0], quat(u[2]));
				break;
			default:
				Log::warn("Uniform type '", u[1], "' not handled by deserialization."); 
				break;
		}
	}
}

#include <typeinfo>

nlohmann::json tojson(const GenericUniform& v)
{
	if(typeid(v) == typeid(const Uniform<int>&))
	{
		auto& u = dynamic_cast<const Uniform<int>&>(v);
		return {u.getName(), "int", u.getValue()};
	} else if(typeid(v) == typeid(const Uniform<float>&)) {
		auto& u = dynamic_cast<const Uniform<float>&>(v);
		return {u.getName(), "float", u.getValue()};
	} else if(typeid(v) == typeid(const Uniform<glm::vec3>&)) {
		auto& u = dynamic_cast<const Uniform<glm::vec3>&>(v);
		return {u.getName(), "vec3", tojson(u.getValue())};
	} else if(typeid(v) == typeid(const Uniform<glm::vec4>&)) {
		auto& u = dynamic_cast<const Uniform<glm::vec4>&>(v);
		return {u.getName(), "vec4", tojson(u.getValue())};
	} else if(typeid(v) == typeid(const Uniform<glm::quat>&)) {
		auto& u = dynamic_cast<const Uniform<glm::quat>&>(v);
		return {u.getName(), "quat", tojson(u.getValue())};
	} else {
		Log::warn("Uniform type '", typeid(v).name(), "' not handled by serialization."); 
	}
	return nlohmann::json{};
}

nlohmann::json tojson(const Material& v)
{
	nlohmann::json r;
	for(const auto& u : v.getUniforms())
	{
		auto serialized = tojson(*u.get());
		if(!serialized.is_null())
			r += serialized;
	}
	return r;
}
