#pragma once

#include <ECS/Entity.hpp>

namespace ecs
{
	
class System
{
public:

private:
	std::bitset<Entity::MaxComponents>	_requirement;
};

};
