#pragma once

// From https://github.com/elanthis/constexpr-hash-demo ("CC0" licensed)

namespace impl
{

// FNV-1a constants
static constexpr unsigned long long basis = 14695981039346656037ULL;
static constexpr unsigned long long prime = 1099511628211ULL;

// compile-time hash helper function
constexpr unsigned long long hash_one(char c, const char* remain, unsigned long long value)
{
	return c == 0 ? value : hash_one(remain[0], remain + 1, (value ^ c) * impl::prime);
}

}

// compile-time hash
constexpr unsigned long long hash(const char* str)
{
	return impl::hash_one(str[0], str + 1, impl::basis);
}

// run-time hash
unsigned long long hash_rt(const char* str)
{
	unsigned long long hash = impl::basis;
	while (*str != 0) {
		hash ^= str[0];
		hash *= impl::prime;
		++str;
	}
	return hash;
}
