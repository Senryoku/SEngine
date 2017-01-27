#pragma once

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

inline nlohmann::json tojson(const glm::vec3& v)
{
	return nlohmann::json{v[0], v[1], v[2]};
}

inline nlohmann::json tojson(const glm::vec4& v)
{
	return nlohmann::json{v[0], v[1], v[2], v[3]};
}

inline nlohmann::json tojson(const glm::quat& v)
{
	return nlohmann::json{v.x, v.y, v.z, v.w};
}
