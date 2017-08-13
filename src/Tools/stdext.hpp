#pragma once
	
#include <sstream>
#include <iomanip>

namespace stdext
{
	
template <typename T>
std::string to_string(const T v, const int n)
{
    std::ostringstream out;
    out << std::setprecision(n) << v;
    return out.str();
}

}