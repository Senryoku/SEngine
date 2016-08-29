#pragma once

#include <vector>
#include <limits>
#include <cassert>

using ComponentID = std::size_t;

constexpr std::size_t max_entities = 2048;
constexpr std::size_t max_component_types = 64;

template<typename T>
std::vector<T>		components;			///< Component storage
template<typename T>
std::vector<bool>	valid_components;	///< Marks components as actively used (Could store the entity index)

////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Types Managment

extern std::size_t next_component_type_idx;
constexpr std::size_t invalid_component_type_idx = std::numeric_limits<std::size_t>::max();

/// Returns a unique index associated to the component type T
template<typename T>
inline std::size_t get_component_type_idx()
{
	static std::size_t component_type_idx = next_component_type_idx++;
	assert(component_type_idx < max_component_types);
	return component_type_idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Component IDs Managment

constexpr ComponentID invalid_component_idx = std::numeric_limits<std::size_t>::max();

template<typename T>
ComponentID next_component_idx = 0;

template<typename T, typename ...Args>
inline ComponentID add_component(Args ...args)
{
	// Makes sure component is allocated
	if(next_component_idx<T> <= components<T>.size())
	{
		components<T>.resize(std::max(static_cast<std::size_t>(64), components<T>.size()) * 2);
		valid_components<T>.resize(std::max(static_cast<std::size_t>(64), components<T>.size()) * 2, false);
	}
	// Search next id
	auto r = next_component_idx<T>++;
	while(next_component_idx<T> < components<T>.size() && valid_components<T>[next_component_idx<T>])
		++next_component_idx<T>;
	
	// Construct and return component
	components<T>[r] = T{std::forward<Args>(args)...};
	valid_components<T>[r] = true;
	return r;
}

template<typename T>
inline void delete_component(ComponentID idx)
{
	components<T>[idx].~T();
	valid_components<T>[idx] = false;
}
