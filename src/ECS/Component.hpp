#pragma once

#include <vector>
#include <list>
#include <limits>
#include <cassert>
#include <iterator>
#include <functional>
#include <iostream>

using ComponentID = std::size_t;
using EntityID = std::size_t;

constexpr EntityID invalid_entity = std::numeric_limits<EntityID>::max();
constexpr ComponentID invalid_component_type_idx = std::numeric_limits<ComponentID>::max();

constexpr std::size_t max_entities = 2048;
constexpr std::size_t max_component_types = 64;

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace impl
{

template<typename T>
class ComponentPool
{
public:
	ComponentPool() :
		_buffer_size{64},
		_data{static_cast<T*>(std::malloc(_buffer_size * sizeof(T)))},
		_owners{static_cast<EntityID*>(std::malloc(_buffer_size * sizeof(EntityID)))}
	{
		for(size_t i = 0; i < size(); ++i)
			_owners[i] = invalid_entity;
	}

	~ComponentPool()
	{
		for(size_t i = 0; i < size(); ++i)
			if(is_valid(i))
				_data[i].~T();	// Explicit destructor if needed
		std::free(_data);
		std::free(_owners);
	}
	
	inline size_t size() const { return _buffer_size; }
	inline T& operator[](ComponentID id) { return _data[id]; }
	inline const T& operator[](ComponentID id) const { return _data[id]; }
	
	inline bool is_valid(ComponentID id) const { return _owners[id] != invalid_entity; }
	inline ComponentID get_id(const T& c) const { return std::distance<const T*>(_data, &c); }
	inline EntityID get_owner(ComponentID id) const { return _owners[id]; }
	
	class iterator : public std::iterator<std::forward_iterator_tag, T>
	{
    public:
        explicit iterator(const ComponentPool& pool, ComponentID idx) :
			_pool{pool},
			_idx{idx > pool.size() ? pool.size() : idx}
		{}
        iterator& operator++()
		{
			do
			{
				++_idx;
			} while(_idx < _pool.size() && _pool.is_valid(_idx));
			assert(_idx <= _pool.size());
			return *this;
		}
        iterator operator++(int) { iterator r = *this; ++(*this); return r; }
        bool operator==(iterator other) const {return _idx == other._idx;}
        bool operator!=(iterator other) const {return !(*this == other);}
        typename std::iterator<std::forward_iterator_tag, T>::reference operator*() const
		{
			assert(_idx < _pool.size());
			return _pool[_idx];
		}
	private:
		const ComponentPool&			_pool;
		ComponentID						_idx;
    };
	
	iterator begin() const
	{
		ComponentID idx = 0;
		while(idx < size() && !is_valid(idx))
			++idx;
		return iterator{*this, idx};
	}
	iterator end() const { return iterator{*this, size()}; }
	
	template<typename ...Args>
	ComponentID add(EntityID eid, Args... args)
	{
		auto id = _next_id;
		
		// Search next valid id.
		do ++_next_id; while(_next_id < _buffer_size && is_valid(_next_id));
		
		// Makes sure buffer is big enough for the next insertion
		if(_next_id >= _buffer_size)
			resize();
		
		// Construct component
		::new(_data + id) T{std::forward<Args>(args)...};
		_owners[id] = eid;
		
		return id;
	}
	
	void rem(ComponentID id)
	{
		_owners[id] = invalid_entity;
		_data[id].~T();
		
		if(id < _next_id)
			_next_id = id;
	}
	
private:
	size_t		_next_id = 0;		/// First invalid id in the array
	size_t		_buffer_size = 0;	/// Buffer size

	T*			_data = nullptr;
	EntityID*	_owners = nullptr;
	
	void resize()
	{
		auto curr_size = _buffer_size;
		_buffer_size *= 2;
		std::cout << "Resize to " << _buffer_size << std::endl;
		
		T* new_buffer = new T[_buffer_size];
		assert(new_buffer);
		for(size_t i = 0; i < curr_size; ++i)
			if(is_valid(i))
				::new(new_buffer + i) T{std::move(_data[i])};	// Explicit move
		std::free(_data);
		_data = new_buffer;
		
		_owners = static_cast<EntityID*>(std::realloc(_owners, _buffer_size * sizeof(EntityID)));
		assert(_owners);
		for(size_t i = curr_size; i < _buffer_size; ++i)
			_owners[i] = invalid_entity;
	}
};

template<typename T>
ComponentPool<T>				components;				///< Component storage

extern std::list<ComponentID>	marked_for_deletion;	///< Components marked for deletion (we don't know their types yet).
extern std::size_t 				next_component_type_idx;

template<typename T>
ComponentID 					next_component_idx = 0;

} // impl namespace
////////////////////////////////////////////////////////////////////////////////////////////////////

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
	return impl::components<T>.get_owner(get_id<T>(c));
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
