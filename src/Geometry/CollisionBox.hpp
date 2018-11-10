#pragma once

#include <serialization.hpp>
#include <BoundingShape.hpp>

class CollisionBox : public BoundingBox {
public:
	CollisionBox() =default;
	CollisionBox(const nlohmann::json& json);
	CollisionBox(CollisionBox&&) =default;
	
	nlohmann::json json() const;
};
