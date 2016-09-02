#pragma once

#include <array>
#include <string>
#include <limits>

#include <Component.hpp>
	
class Entity
{
public:	
	Entity()
	{
		for(auto& id : _components)
			id = invalid_component_idx;
	}
	
	Entity(const Entity&) =delete;
	Entity(Entity&& old) :
		_id{old._id},
		_name{old._name},
		_components{old._components}
	{	
		old.invalidate();
	}
	
	Entity& operator=(const Entity&) =delete;
	Entity& operator=(Entity&&) =default;
	
	Entity(EntityID eid) :
		_id{eid},
		_name{std::to_string(eid)}
	{
		for(auto& cid : _components)
			cid = invalid_component_idx;
	}
	
	~Entity()
	{
		if(is_valid())
			for(auto& id : _components)
				if(id != invalid_component_idx)
					mark_for_deletion(id);
		invalidate();
	}
	
	inline std::string get_name() const { return _name; }
	inline void set_name(const std::string& n) { _name = n; }

	inline EntityID get_id() const { return _id; }
	
	template<typename T>
	inline bool has()
	{
		return _components[get_component_type_idx<T>()] != invalid_component_idx;
	}
	
	template<typename T>
	inline T& get()
	{
		assert(has<T>());
		return impl::components<T>[_components[get_component_type_idx<T>()]];
	}
	
	template<typename T>
	inline T& get_id()
	{
		assert(has<T>());
		return _components[get_component_type_idx<T>()];
	}
	
	template<typename T, typename ...Args>
	inline T& add(Args&& ...args)
	{
		assert(_id != invalid_entity);
		if(has<T>())
		{
			impl::components<T>[_components[get_component_type_idx<T>()]] = T{std::forward<Args>(args)...};
		} else {
			_components[get_component_type_idx<T>()] = add_component<T>(_id, std::forward<Args>(args)...);
			assert(has<T>());
		}
		return impl::components<T>[_components[get_component_type_idx<T>()]];
	}
	
	template<typename T>
	inline void rem()
	{
		assert(has<T>());
		mark_for_deletion(_components[get_component_type_idx<T>()]);
		_components[get_component_type_idx<T>()] = invalid_component_idx;
	}
	
	inline bool is_valid() const
	{
		return _id != invalid_entity;
	}
	
private:
	EntityID										_id = invalid_entity;
	std::string										_name;
	std::array<ComponentID, max_component_types>	_components;
	
	void invalidate()
	{
		if(is_valid())
			for(auto& id : _components)
				id = invalid_component_idx;
		_id = invalid_entity;
		_name = "";
	}
};

extern EntityID				next_entity_id;
extern std::vector<Entity>	entities;

inline Entity& get_entity(EntityID id)
{
	return entities[id];
}

inline Entity& create_entity()
{
	// Allocating if necessary
	if(next_entity_id + 1 >= entities.size())
		entities.resize(std::max(static_cast<size_t>(64), entities.size() * 2));
	
	auto r = next_entity_id++;
	// Searching for the next id
	while(entities[next_entity_id].is_valid())
		++next_entity_id;

	// Constructing and returning the entity
	::new(&entities[r]) Entity{r};
	return entities[r];
}

inline void destroy_entity(EntityID id)
{
	entities[id].~Entity();
	if(id < next_entity_id)
		next_entity_id = id;
}
