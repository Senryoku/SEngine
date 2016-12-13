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

inline glm::quat quat(const nlohmann::json& j)
{
	assert(j.is_array());
	return glm::quat{j[3], j[0], j[1], j[2]};
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
		r = glm::translate(glm::mat4(1.0f), vec3(json["position"])) * 
			glm::mat4_cast(quat(json["rotation"])) * 
			glm::scale(glm::mat4(1.0f), vec3(json["scale"]));
	}
	return r;
}

inline nlohmann::json tojson(const glm::vec3& v)
{
	return nlohmann::json{v[0], v[1], v[2]};
}

inline nlohmann::json tojson(const glm::quat& v)
{
	return nlohmann::json{v.x, v.y, v.z, v.w};
}
