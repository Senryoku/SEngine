#pragma once

#include <functional>
#include <iterator>

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
			assert(is_valid<T>(_idx));
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
