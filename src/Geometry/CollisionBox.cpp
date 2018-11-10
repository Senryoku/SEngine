#include <CollisionBox.hpp>

CollisionBox::CollisionBox(const nlohmann::json& json) :
	BoundingBox(vec3(json["min"]), vec3(json["max"]))
{
}

nlohmann::json CollisionBox::json() const {
	return {
		{"min", tojson(min)},
		{"max", tojson(max)}
	};
}
