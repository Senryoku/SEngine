#pragma once

template<typename L, typename R>
struct Equal
{
	static constexpr bool value = false;
};

template<typename L>
struct Equal<L, L>
{
	static constexpr bool value = true;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// List

template<typename... T>
struct TList
{
};

////////////////////////////////////////////////

template<typename L>
struct count;

template<typename Head, typename... Tail>
struct count<TList<Head, Tail...>>
{
	static constexpr int value = 1 + count<TList<Tail...>>::value;
};

template<>
struct count<TList<>>
{
	static constexpr int value = 0;
};

////////////////////////////////////////////////

template<typename T, typename L>
struct index_of;

// 
template<typename T, typename Head, typename... Tail>
struct index_of<T, TList<Head, Tail...>>
{
	static constexpr int next_value = index_of<T, TList<Tail...>>::value;
	static constexpr int value = next_value >= 0 ? next_value + 1 : -1;
};

// Specialization : T == Head
template<typename T, typename... Tail>
struct index_of<T, TList<T, Tail...>>
{
	static constexpr int value = 0;
};

// Specialization : No more Tail (T not found)
template<typename T>
struct index_of<T, TList<>>
{
	static constexpr int value = -1;
};

////////////////////////////////////////////////
template<typename T, typename L>
struct includes;

template<typename T, typename Head, typename... Tail>
struct includes<T, TList<Head, Tail...>>
{
	static constexpr int value = index_of<T, TList<Head, Tail...>>::value > -1;
};

////////////////////////////////////////////////

template<int N, typename L>
struct nth;

// N > 0, recursion
template<int N, typename Head, typename... Tail>
struct nth<N, TList<Head, Tail...>>
{
	using value = typename nth<N - 1, TList<Tail...>>::value;
};

// Specialization : Found the nth type, return it.
template<typename Head, typename... Tail>
struct nth<0, TList<Head, Tail...>>
{
	using value = Head;
};

////////////////////////////////////////////////

template<template<typename> typename F, typename L>
struct for_each;

template<template<typename> typename F, typename Head, typename... Tail>
struct for_each<F, TList<Head, Tail...>>
{
	void operator()() const
	{
		F<Head>{}();
		for_each<F, TList<Tail...>>{}();
	}
};

template<template<typename> typename F>
struct for_each<F, TList<>>
{
	void operator()() const
	{
	}
};

////////////////////////////////////////////////
