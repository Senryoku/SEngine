#pragma once

#include <json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <Log.hpp>

inline glm::vec3 vec3(const nlohmann::json& j);
inline glm::vec4 vec4(const nlohmann::json& j);
inline glm::quat quat(const nlohmann::json& j);
glm::mat4 mat4(const nlohmann::json& json);

inline nlohmann::json tojson(const glm::vec3& v);
inline nlohmann::json tojson(const glm::vec4& v);
inline nlohmann::json tojson(const glm::quat& v);

#include <Uniform.hpp>
#include <Material.hpp>

//Material material(const nlohmann::json& v); // Probably unnecessary
void update_material(const nlohmann::json& v, Material& m);

nlohmann::json tojson(const GenericUniform& v);
nlohmann::json tojson(const Material& v);

#include <serialization.inl>
