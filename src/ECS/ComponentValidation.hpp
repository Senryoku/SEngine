#pragma once

#include <Component.hpp>
#include <Entity.hpp>

template<typename T>
inline bool deep_validation(ComponentID idx)
{
	return is_valid<T>(idx) && is_valid<T>(impl::component_owner<T>[idx]) &&
		entities[impl::component_owner<T>[idx]].template has<T>() &&
		&entities[impl::component_owner<T>[idx]].template get<T>() == &impl::components<T>[idx];
}

template<typename T>
inline void deletion_pass()
{
	auto it = impl::marked_for_deletion.begin();
	while(it != impl::marked_for_deletion.end())
	{
		// Marked as valid, but is not anymore
		if(is_valid<T>(*it) && !deep_validation<T>(*it))
		{
			delete_component<T>(*it);
			it = impl::marked_for_deletion.erase(it);
		} else {
			++it;
		}
	}
}
