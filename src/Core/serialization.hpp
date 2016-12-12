#pragma once

#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

inline glm::vec3 vec3(const nlohmann::json& j)
{
	assert(j.is_array());
	return glm::vec3{j[0], j[1], j[2]};
}

inline glm::vec4 vec4(const nlohmann::json& j)
{
	assert(j.is_array());
	return glm::vec4{j[0], j[1], j[2], j[3]};
}

inline glm::mat4 mat4(const nlohmann::json& json)
{
	glm::mat4 r;
	if(json.is_array())
	{
		for(int i = 0; i < 4; ++i)
			for(int j = 0; j < 4; ++j)
				r[i][j] = json[i * 4 + j];
	} else {
		r = glm::scale(
				glm::translate(
					glm::rotate(static_cast<float>(json["rotation"][0]), glm::vec3{1, 0, 0}) *  
					glm::rotate(static_cast<float>(json["rotation"][1]), glm::vec3{0, 1, 0}) *
					glm::rotate(static_cast<float>(json["rotation"][2]), glm::vec3{0, 0, 1})
				, vec3(json["position"])), 
				vec3(json["scale"]));
	}
	return r;
}

inline nlohmann::json tojson(const glm::vec3& v)
{
	return nlohmann::json {v[0], v[1], v[2]};
}

inline nlohmann::json tojson(const glm::quat& v)
{
	return nlohmann::json {v[0], v[1], v[2], v[3]};
}
