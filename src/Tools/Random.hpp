#include <chrono>
#include <random>
#include <array>
#include <iostream>

#include <glm/glm.hpp>

#include <Singleton.hpp>
#include <MathTools.hpp>

/// @todo static ?

class RandomHelper : public Singleton<RandomHelper>
{
public:
	std::mt19937										generator;
	std::uniform_real_distribution<double>	distribution0_1;
	
	RandomHelper() :
		generator(std::chrono::system_clock::now().time_since_epoch().count()),
		distribution0_1(0.0, 1.0)
	{
	}
	
	double get0_1()
	{
		return distribution0_1(generator);
	}
	
	/// Uniform distribution
	inline glm::vec2 getSphericalAngles()
	{
		return glm::vec2{2.0 * pi() * get0_1(), std::acos(2.0 * get0_1() - 1.0)};
	}
	
	/// Uniform distribution
	inline glm::vec3 getSpherical()
	{
		auto a = getSphericalAngles();
		auto s0 = std::sin(a[0]);
		return glm::vec3{s0 * std::cos(a[1]),
							s0* std::sin(a[1]),
							std::cos(a[0])};
	}
	
	/// Uniform distribution
	inline glm::vec3 getHemispherical(const glm::vec3& n)
	{
		auto r = getSpherical();
		if(glm::dot(r, n) < 0.0)
			r = -r;
		return r;
	}
};