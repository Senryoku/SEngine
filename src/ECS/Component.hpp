#pragma once

#include <vector>
#include <list>
#include <limits>
#include <cassert>
#include <iterator>
#include <functional>

using ComponentID = std::size_t;
using EntityID = std::size_t;

constexpr EntityID invalid_entity = std::numeric_limits<EntityID>::max();
constexpr ComponentID invalid_component_type_idx = std::numeric_limits<ComponentID>::max();

constexpr std::size_t max_entities = 2048;
constexpr std::size_t max_component_types = 64;

namespace impl
{
	
template<typename T>
std::vector<T>			components;			///< Component storage
template<typename T>
std::vector<EntityID>	component_owner;	///< Marks components as actively used.

extern std::list<ComponentID>	marked_for_deletion;	///< Components marked for deletion (we don't know their types yet).

extern std::size_t next_component_type_idx;

template<typename T>
ComponentID next_component_idx = 0;

} // impl namespace

template<typename T>
inline bool is_valid(ComponentID idx)
{
	return impl::component_owner<T>[idx] != invalid_entity;
}

template<typename T>
inline ComponentID get_id(const T& c)
{
	return std::distance(&*impl::components<T>.cbegin(), &c);
}

template<typename T>
inline EntityID get_owner(ComponentID idx)
{
	return impl::component_owner<T>[idx];
}

template<typename T>
inline EntityID get_owner(const T& c)
{
	return impl::component_owner<T>[get_id<T>(c)];
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
	// Makes sure component is allocated
	if(impl::next_component_idx<T> + 1 >= impl::components<T>.size())
	{
		/// @todo ANOTHER BIG PROBLEM...
		/// This does not only allocates memory for the components, but also construct them, and we
		/// don't want that... I should be using something else than std::vector, but there is
		/// something wrong with my tentatives of implementations ~~
		auto target_size = std::max(static_cast<std::size_t>(64), impl::components<T>.size() * 2);
		impl::component_owner<T>.resize(target_size, invalid_entity);
		impl::components<T>.resize(target_size);
	}
	// Search next id
	auto r = impl::next_component_idx<T>++;
	while(impl::next_component_idx<T> < impl::components<T>.size() && is_valid<T>(impl::next_component_idx<T>))
		++impl::next_component_idx<T>;
	
	/// @todo HACK
	/// Explicitly calls the destructor of the previous component. We know it exists, since
	/// std::vector constructs them by default... However, we would like to avoid that =/
	impl::components<T>[r].~T();

	// Construct and return component
	impl::component_owner<T>[r] = eid;
	::new(&impl::components<T>[r]) T{std::forward<Args>(args)...};
	return r;
}

inline void mark_for_deletion(ComponentID idx)
{
	impl::marked_for_deletion.push_back(idx);
}

template<typename T>
inline void delete_component(ComponentID idx)
{
	// To call the component destructor here, we should forbid the destructor calls at exit
	// (via std::vector) and manage them ourselves...
	//impl::components<T>[idx].~T();
	/// @todo THIS IS A SERIOUS PROBLEM!!!!
	///		  See add_component for the current hackish solution, and what we should be doing...
	impl::component_owner<T>[idx] = invalid_entity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Utils

/**
 * Use this to iterates over all valid instances of a component type.
 * The predicate argument can be used to work only on a subset of all valid instances (only the
 * instances for which predicate returns true will be considered).
**/
template<typename T>
class ComponentIterator
{
public:
	
	ComponentIterator(const std::function<bool(const T&)>& predicate = [](const T&) -> bool { return true; }) : 
		_predicate{predicate}
	{
	}
	
	class iterator : public std::iterator<std::forward_iterator_tag, T>
	{
    public:
        explicit iterator(std::function<bool(const T&)> predicate, ComponentID idx = impl::components<T>.size()) :
			_predicate{predicate},
			_idx{idx > impl::components<T>.size() ? impl::components<T>.size() : idx}
		{}
        iterator& operator++()
		{
			do
			{
				++_idx;
			} while(_idx < impl::components<T>.size() && (!is_valid<T>(_idx) || !_predicate(impl::components<T>[_idx])));
			assert(_idx <= impl::components<T>.size());
			return *this;
		}
        iterator operator++(int) { iterator r = *this; ++(*this); return r; }
        bool operator==(iterator other) const {return _idx == other._idx;}
        bool operator!=(iterator other) const {return !(*this == other);}
        typename std::iterator<std::forward_iterator_tag, T>::reference operator*() const
		{
			assert(_idx < impl::components<T>.size());
			return impl::components<T>[_idx];
		}
	private:
		std::function<bool(const T&)>	_predicate;
		ComponentID						_idx;
    };
	
	iterator begin() const
	{
		ComponentID idx = 0;
		while(idx < impl::components<T>.size() && (!is_valid<T>(idx) || !_predicate(impl::components<T>[idx])))
			++idx;
		return iterator{_predicate, idx};
	}
	iterator end() const { return iterator{_predicate}; }
private:
	std::function<bool(const T&)> _predicate;
};
