#pragma once

#include <cmath>
#include <limits>

/// Pi Constant
constexpr double pi() { return std::atan(1)*4; }

/**
 * @param val
 * @param min
 * @param max
 * @return Clamped value between min and max
**/
template<typename ScalarType>
ScalarType clamp(ScalarType val, ScalarType min, ScalarType max)
{
	if(val < min) return min;
	else if(val > max) return max;
	return val;
}

/**
 * Kind of a generic mod
**/
template<typename ScalarType>
ScalarType wrap(ScalarType val, ScalarType min = ScalarType(), ScalarType max = static_cast<ScalarType>(1.f))
{
	while(val < min) val += max - min;
	while(val > max) val -= max - min;
}

/**
 * Generic mod for floating-point types.
 * @param x
 * @param y
 * @return x % y
**/
template<typename T>
T mod(T x, T y)
{
    static_assert(!std::numeric_limits<T>::is_exact , "mod: floating-point type expected");

    if (0. == y)
        return x;

    double m = x - y * floor(x/y);

    if(y > 0)
    {
        if(m >= y)
            return 0;
			
        if(m < 0)
        {
            if (y + m == y)
                return 0;
            else
				return y + m;
        }
    } else {
        if(m <= y)
            return 0;

        if(m > 0)
        {
            if (y + m == y)
                return 0;
            else
                return y + m;
        }
    }

    return m;
}

template<typename T>
T triangleWave(T t, T p)
{
	return std::abs(2.0 * (t / p - std::floor(t / p + 0.5)));
}

template<typename T>
T sqr(T a)
{
	return a * a;
}

template<typename T>
T max(std::initializer_list<T> args)
{
	T r = std::numeric_limits<T>::min();
	for(const auto& v: args)
		r = std::max(v, r);
	return r;
}

template<typename T>
T min(std::initializer_list<T> args)
{
	T r = std::numeric_limits<T>::max();
	for(const auto& v: args)
		r = std::min(v, r);
	return r;
}
