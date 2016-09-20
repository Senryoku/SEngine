#pragma once

#include <cmath>
#include <vector>
#include <functional>

template<typename Val>
class Bezier
{
	public:
		Bezier() =default;
		Bezier(const Bezier<Val>&) =default;
	
		Bezier(std::vector<Val>& controlPoints, unsigned int nb_points = 100) :
			_control_points(controlPoints)
		{
			compute(nb_points);
		}
		
		~Bezier() {}

		void compute(unsigned int nb_points = 100)
		{
			_points.clear();
			_points.reserve(nb_points);
			for(unsigned int i(0); i < nb_points; i++)
			{
				Val P{0};
				for(unsigned int j(0); j < _control_points.size(); j++)
					P += _control_points[j] * berstein(_control_points.size() - 1, j,
						static_cast<float>(i)/static_cast<float>(nb_points));
				_points.push_back(P);
			}
		}

		unsigned int getControlPointCount() { return _control_points.size(); }
		unsigned int getPointCount() { return _points.size(); }

		Val get(unsigned int nPoint) { return _points[std::min(nPoint, _points.size() - 1)]; }
		Val operator[](unsigned int nPoint) { return _points[std::min(nPoint, _points.size() - 1)]; }
		Val operator()(unsigned int nPoint) { return _control_points[std::min(nPoint, _control_points.size() - 1)]; }

		Bezier<Val>& operator+=(Val v) { _control_points.push_back(v); }
		
		void reset() { _control_points.clear(); }

		Val getDelta(unsigned int nPoint)
		{
			nPoint = clamp(nPoint, 0, _points.size() - 1);
			return _points[nPoint + 1] - _points[nPoint];
		}

		Val get(float t)
		{
			t = clamp(t, 0.0f, 1.0f);
			return _points[static_cast<unsigned int>(t * _points.size())];
		}
		
	private:
		std::vector<Val> _control_points;		///< Control Points of the curve
		std::vector<Val> _points;				///< Pre-Computed points

		template<class T, class Compare>
		constexpr const T& clamp( const T& v, const T& lo, const T& hi, Compare comp )
		{
			return comp(v, hi) ? std::max(v, lo, comp) : std::min(v, hi, comp);
		}

		template<class T>
		constexpr const T& clamp( const T& v, const T& lo, const T& hi )
		{
			return clamp(v, lo, hi, std::less<>());
		}
		
		constexpr unsigned int factorial(unsigned int n)
		{
			return (n < 2) ? 1 : n * factorial(n - 1);
		}
		
		inline unsigned int binomial(unsigned int n, unsigned int k)
		{
			return (k <= n) ? factorial(n) / (factorial(k) * factorial(n - k)) : 0;
		}

		inline float berstein(unsigned int m, unsigned int i, float u)
		{
			return binomial(m, i) * pow(u, i) * pow(1 - u, m - i);
		}
};
