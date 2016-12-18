#pragma once

/**
 * ComponentPool
 * Preallocated buffer of memory to hold components of type T
**/
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
	inline size_t count() const { return _count; }
	inline T& operator[](ComponentID id) { return _data[id]; }
	inline const T& operator[](ComponentID id) const { return _data[id]; }
	
	inline bool is_valid(ComponentID id) const { return _owners[id] != invalid_entity; }
	
	inline ComponentID get_id(const T& c) const
	{
		auto d = std::distance<const T*>(_data, &c);
		assert(d >= 0);
		assert(d < static_cast<decltype(d)>(size()));
		return d;
	}
	
	inline EntityID get_owner(ComponentID id) const
	{
		assert(id < size());
		return _owners[id];
	}
	
	class iterator : public std::iterator<std::forward_iterator_tag, T>
	{
    public:
        explicit iterator(const ComponentPool& pool, ComponentID idx) :
			_pool{pool},
			_idx{idx > pool.size() ? pool.size() : idx}
		{}
        iterator& operator++()
		{
			do ++_idx; while(_idx < _pool.size() && _pool.is_valid(_idx));
			assert(_idx <= _pool.size());
			return *this;
		}
        iterator operator++(int) { iterator r = *this; ++(*this); return r; }
        bool operator==(iterator other) const {return _idx == other._idx;}
        bool operator!=(iterator other) const {return !(*this == other);}
        typename std::iterator<std::forward_iterator_tag, T>::reference operator*() const
		{
			assert(_idx < _pool.size());
			assert(is_valid(_idx));
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
	ComponentID add(EntityID eid, Args&&... args)
	{
		auto id = _next_id;
		
		// Search next valid id.
		do ++_next_id; while(_next_id < _buffer_size && is_valid(_next_id));
		
		// Makes sure buffer is big enough for the next insertion
		if(_next_id >= _buffer_size)
			resize();
		
		// Construct component
		_owners[id] = eid;
		::new(_data + id) T{std::forward<Args>(args)...};
		
		++_count;
		
		return id;
	}
	
	template<typename ...Args>
	void replace(ComponentID id, Args&&... args)
	{
		assert(is_valid(id));
		_data[id].~T();
		::new(_data + id) T{std::forward<Args>(args)...};
	}
	
	void rem(ComponentID id)
	{
		_owners[id] = invalid_entity;
		_data[id].~T();
		
		if(id < _next_id)
			_next_id = id;
		
		--_count;
	}
	
private:
	size_t		_next_id = 0;		/// First invalid id in the array
	size_t		_buffer_size = 0;	/// Buffer size
	size_t		_count = 0;			/// Used slot count

	T*			_data = nullptr;
	EntityID*	_owners = nullptr;
	
	void resize()
	{
		auto curr_size = _buffer_size;
		_buffer_size *= 2;
		
		T* new_buffer = static_cast<T*>(std::malloc(_buffer_size * sizeof(T)));
		assert(new_buffer);
		for(size_t i = 0; i < curr_size; ++i)
			if(is_valid(i))
			{
				::new(new_buffer + i) T{std::move(_data[i])};	// Explicit move
				_data[i].~T();
			}
		std::free(_data);
		_data = new_buffer;
		
		_owners = static_cast<EntityID*>(std::realloc(_owners, _buffer_size * sizeof(EntityID)));
		assert(_owners);
		for(size_t i = curr_size; i < _buffer_size; ++i)
			_owners[i] = invalid_entity;
	}
};
