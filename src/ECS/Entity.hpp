#pragma once

#include <array>
#include <limits>

#include <Component.hpp>

using EntityID = std::size_t;
	
class Entity
{
public:
	constexpr static EntityID InvalidID = std::numeric_limits<EntityID>::max();
	
	Entity()
	{
		for(auto& id : _components)
			id = invalid_component_idx;
	}
	
	~Entity()
	{
		/// @todo Find a way to call the destructor of a component?
		/*
		if(is_valid())
			for(auto id : _components)
				if(id != invalid_component_idx)
					//delete_component<>(id);
		*/
	}
	
	template<typename T>
	inline T& get()
	{
		assert(_components[get_component_type_idx<T>()] != invalid_component_idx);
		return components<T>[_components[get_component_type_idx<T>()]];
	}
	
	template<typename T, typename ...Args>
	inline T& add(Args ...args)
	{
		if(_components[get_component_type_idx<T>()] != invalid_component_idx)
		{
			components<T>[_components[get_component_type_idx<T>()]] = T{std::forward<Args>(args)...};
		} else {
			_components[get_component_type_idx<T>()] = add_component<T>(std::forward<Args>(args)...);
		}
		return components<T>[_components[get_component_type_idx<T>()]];
	}
	
	inline bool is_valid() const
	{
		return _id != InvalidID;
	}
	
private:
	EntityID										_id = InvalidID;
	std::array<ComponentID, max_component_types>	_components;
};

extern EntityID				next_entity_id;
extern std::vector<Entity>	entities;

inline Entity create_entity()
{
	if(next_entity_id <= entities.size())
		entities.resize(std::max(static_cast<size_t>(64), entities.size()) * 2);
	auto r = next_entity_id++;
	while(entities[next_entity_id].is_valid())
		++next_entity_id;
	return entities[r];
}
