#pragma once

#include <cmath>
#include <limits>
#include <algorithm>
#include <initializer_list>

/// Pi Constant
constexpr double pi() { return std::atan(1)*4; }

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

namespace stdext {
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

template<typename T>
T max(T h, T t)
{
	return h > t ? h : t;
}

template<typename H, typename... T>
H max(H h, T... t)
{
	return max(h, max(t...));
}

template<typename T>
T min(T h, T t)
{
	return h < t ? h : t;
}

template<typename H, typename... T>
H min(H h, T... t)
{
	return min(h, min(t...));
}

/**
 * @return The next value representable in this type.
**/
template<typename T>
T next_representable(const T& val)
{
	return std::nextafter(val, std::numeric_limits<T>::max());
}

}
