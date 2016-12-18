#pragma once

#include <vector>
#include <list>
#include <limits>
#include <cassert>

/**
 * (Almost) Any type can be used as a component, just add one to an entity.
 * Once added to a entity, pointers to a component can be invalidated after another insertion
 * (so we can have a dynamically sized buffer), it is therefore not wise to refer to them this way,
 * prefer using their ComponentID (and retrieve them with the get_component<T>(ComponentID) function).
 * For the same reason, the component type must provide a correct move constructor (they can be
 * moved around after an insertion).
**/

using ComponentID = std::size_t;
using EntityID = std::size_t;

constexpr EntityID invalid_entity = std::numeric_limits<EntityID>::max();
constexpr ComponentID invalid_component_type_idx = std::numeric_limits<ComponentID>::max();

constexpr std::size_t max_entities = 2048;
constexpr std::size_t max_component_types = 64;

#include <ComponentPool.hpp> // Eeeeeeh

namespace impl
{

template<typename T>
ComponentPool<T>				components;				///< Component storage

extern std::list<ComponentID>	marked_for_deletion;	///< Components marked for deletion (we don't know their types yet).

extern std::size_t 				next_component_type_idx;///< First unused component type index

template<typename T>
ComponentID 					next_component_idx = 0;	///< First unused ComponentID

} // impl namespace

template<typename T>
inline bool is_valid(ComponentID idx)
{
	return impl::components<T>.is_valid(idx);
}

template<typename T>
inline ComponentID get_id(const T& c)
{
	return impl::components<T>.get_id(c);
}

template<typename T>
inline EntityID get_owner(ComponentID idx)
{
	return impl::components<T>.get_owner(idx);
}

template<typename T>
inline EntityID get_owner(const T& c)
{
	return impl::components<T>.get_owner(impl::components<T>.get_id(c));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Component Types Managment

/// Returns a unique index associated to the component type T
template<typename T>
inline std::size_t get_component_type_idx()
{
	static std::size_t component_type_idx = impl::next_component_type_idx++;
	assert(component_type_idx < max_component_types);
	return component_type_idx;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Component IDs Managment

constexpr ComponentID invalid_component_idx = std::numeric_limits<std::size_t>::max();

template<typename T>
inline T& get_component(ComponentID id)
{
	assert(id < impl::components<T>.size());
	return impl::components<T>[id];
}

template<typename T, typename ...Args>
inline ComponentID add_component(EntityID eid, Args&& ...args)
{
	return impl::components<T>.add(eid, std::forward<Args>(args)...);
}

inline void mark_for_deletion(ComponentID idx)
{
	impl::marked_for_deletion.push_back(idx);
}

template<typename T>
inline void delete_component(ComponentID idx)
{
	impl::components<T>.rem(idx);
}

#include <ComponentIterator.hpp>
