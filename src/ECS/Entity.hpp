#pragma once

#include <bitset>

#include <ECS/World.hpp>

namespace ecs
{
	
class Entity
{
public:
	static constexpr size_t MaxComponents = 64;
	
	Entity() :
		_id(createEntity())
	{
	}
	
	EntityID id() const
	{
		return _id;
	}
	
	template<typename T, typename ... Args>
	void add(Args&& ... args)
	{
		assign(_id, T{std::forward<Args>(args)...});
	}
	
private:
	EntityID					_id;
	
	std::bitset<MaxComponents>	_components;
};

}
