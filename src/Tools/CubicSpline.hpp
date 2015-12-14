#pragma once

#include <array>
#include <vector>
	
/** 
 * @brief CubicSpline
 * @author Senryoku 
 *
 * Senryoku <https://github.com/Senryoku>
 * (Started on October 2014)
 *
 * Describes a CubicSpline defined by ControlPoints.
 * Each ControlPoint consist of at least a position, and
 * optionally a speed (tangent) and a time (these two can be set to
 * default values, giving you a [0, 1] Catmull-Rom spline).
 * The spline will reach each of its ConstrolPoint's positions 
 * at the given time and speed.
 * Some of it is precomputed to speed up at use, any modification
 * other than adding a point (which causes a automatic update) 
 * should be followed by a call to update()
 * @see void update()
 *
 * @todo Easier point insertion. Auto sorting if time is specified ?
 * @todo Handle more use cases (than just default linear/Catmull-Rom)
 *       especially while inserting ControlPoints.
 * @todo Comment the whole thing
 *
 * @param T Well defined vector class (addition, multiplication with a scalar R)
 * @param R Scalar type of the vector class (default: float)
**/
template<class T, typename R = float>
class CubicSpline
{
public:	
	using vector_type = T;
	using scalar_type = R;

	/**
	 * @brief ControlPoint
	 *
	 * Describes a point in space (T) where an object following the 
	 * spline should be at a given time (R), at a given speed (T). 
	 * Speed and Time can be omitted, in this case, the CubicSpline
	 * class will use default values for these two properties.
	**/
	class ControlPoint
	{
	public:
		ControlPoint() =default; ///< @brief Default Constructor
		ControlPoint(const ControlPoint&) =default; ///< @brief Copy Constructor
		ControlPoint(ControlPoint&&) =default; ///< @brief Move Constructor
		
		/**
		 * @brief Constructor
		 * 
		 * @param position Position in space
		 * @param speed Speed
		 * @param time Time Spline(time) will return position
		**/
		ControlPoint(T position, T speed = T(), R time = R()) :
			_position(position),
			_speed(speed),
			_time(time)
		{ }
		
		~ControlPoint() =default;
		
		/// @return Position in space of the point
		inline const T& getPosition() const { return _position; }
		
		/// @return Speed (First derivative) of the spline at this point
		inline const T& getSpeed() const { return _speed; }
		
		/// @return Time such as Spline(Time) = Position of this ControlPoint
		inline const R& getTime() const { return _time; }
		
		/// @param v New Position
		inline void setPosition(const T& v) { _position = v; }
		
		/// @param v New Speed
		inline void setSpeed(const T& v) { _speed = v; }
		
		/// @param v New Time
		inline void setTime(const R& v) { _time = v; }
	private:
		T		_position;			///< Position
		T		_speed;				///< Speed (First derivative)
		R		_time;				///< Time
	};
	
	/**
	 * @brief Describes automatic timing strategies
	 *
	**/
	enum Timing
	{
		Linear, ///< Linear between 0.0 and 1.0
		LinearBetweenPoints ///< Linear between 0.0 and getPointCount()
	};
	
	/**
	 * @brief Describes automatic tangents strategies
	 *
	**/
	enum Tangent
	{
		CatmullRom ///< Catmull-Rom-like tangents, with nil acceleration at start and end.
	};
	
	// Constructors
	
	/** @brief Default Constructor **/
	CubicSpline() =default;
	/** @brief Copy Constructor **/
	CubicSpline(const CubicSpline<T, R>&) =default;
	/** @brief Move Constructor **/
	CubicSpline(CubicSpline<T, R>&&) =default;

	/** 
	 * @brief Constructor by list of positions 
	 *
	 * This will construct a Catmull-Rom spline.
	 * @param l List of positions
	 * @see template<class Iterator> CubicSpline(Iterator begin, Iterator end)
	**/
	CubicSpline(const std::initializer_list<T>& l);

	/** @brief Constructor by list of ControlPoints
	 * @param l List of ControlPoints
	**/
	CubicSpline(const std::initializer_list<ControlPoint>& l);

	/** @brief Constructor by list of positions 
	 *
	 * This will construct a Catmull-Rom spline.
	 * @param begin Iterator pointing to the first position
	 * @param end Iterator pointing right after the last position
	 * @see CubicSpline(const std::initializer_list<T>& l)
	**/
	template<class Iterator>
	CubicSpline(Iterator begin, Iterator end);
	
	/// @brief Destructor
	~CubicSpline() =default;
	
	/// @brief Assignment operator
	CubicSpline& operator=(const CubicSpline&) =default;
	
	/// @brief Move operator
	CubicSpline& operator=(CubicSpline&&) =default;
	
	/**
	 * @brief Adds c at the end of the spline.
	 *
	 * ConstrolPoint c must be completely defined (whit tangent and time),
	 * or completed by call(s) to computeTimings/computeTangents.
	 * @param c ControlPoint to add.
	 * @see add(const ControlPoint&)
	**/
	inline void operator+=(const ControlPoint& c) { add(c); }

	/**
	 * @brief Adds c at the end of the spline.
	 *
	 * ConstrolPoint c must be completely defined (whit tangent and time),
	 * or completed by call(s) to computeTimings/computeTangents.
	 * @param c ControlPoint to add.
	 * @see operator+=(const ControlPoint&)
	**/
	inline void add(const ControlPoint& c);

	/// @return The number of ControlPoint describing the spline
	inline size_t getPointCount() const { return _points.size(); }
	
	/**
	 * @return First correct value for get*(R t)
	 * @see get(R T)
	 * @see getEndTime()
	**/
	inline R getStartTime() const { return _points[0].getTime(); }
	
	/**
	 * @return Highest correct value for get*(R t)
	 * @see get(R T)
	 * @see getStartTime()
	**/
	inline R getEndTime() const { return _points[getPointCount() - 1].getTime(); }
	
	/**
	 * @param t Time
	 * @return Value of the spline at t
	 * @see get(R t)
	**/
	inline T operator()(R t) const;

	/**
	 * @param t Time
	 * @return Value of the spline at t
	 * @see operator()(R t)
	**/
	inline T get(R t) const;
	
	/**
	 * @param t Time
	 * @return Value of the speed (first derivative) of the spline at t
	**/
	inline T getSpeed(R t) const;
	
	/**
	 * @param t Time
	 * @return Value of the acceleration (second derivative) of the spline at t
	**/
	inline T getAcceleration(R t) const;
	
	/**
	 * @brief Modify the ControlPoint's times.
	 *
	 * If the spline is described by 3 ControlPoints C1, C2, C3 
	 * with type = Linear and m = 1.0 (default values):\n
	 * C1 will be reached for t = 0.0 * m / 2.0 = 0.0\n
	 * C2 will be reached for t = 1.0 * m / 2.0 = 0.5\n
	 * C3 will be reached for t = 2.0 * m / 2.0 = 1.0
	 * @param type Timing type to use
	 * @param m Maximum time (i.e. getEndTime() will return m)
	**/
	inline void computeTimings(Timing type = Timing::Linear, R m = static_cast<R>(1.0));
	
	/**
	 * @brief Computes tangents at ControlPoints.
	 *
	 * Compute ControlPoint's speeds (~tangents/derivatives)
	 * following the specified strategy.
	 * @param t Strategy to use (default: Catmull-Rom)
	**/
	inline void computeTangents(Tangent t = Tangent::CatmullRom);
	
	/** @brief Used to iterate over ControlPoints
	 *
	 * Be careful using this: any modification to a ControlPoint
	 * needs to be followed by a call to update() !
	 * @see end()
	**/
	inline typename std::vector<ControlPoint>::iterator begin() { return _points.begin(); }
	/**
	 * @brief Used to iterate over ControlPoints
	 *
	 * Be careful using this: any modification to a ControlPoint
	 * needs to be followed by a call to update() !
	 * @see begin()
	**/
	inline typename std::vector<ControlPoint>::iterator end() { return _points.end(); }
	
	/** @brief Used to iterate over const ControlPoints
	 *
	 * @see cend()
	**/
	inline typename std::vector<ControlPoint>::const_iterator cbegin() const { return _points.cbegin(); }
	/**
	 * @brief Used to iterate over const ControlPoints
	 *
	 * @see cbegin()
	**/
	inline typename std::vector<ControlPoint>::const_iterator cend() const { return _points.cend(); }
	
	/**
	 * @brief Updates internal representation of the spline
	 *
	 * Should be called after each modification of a ControlPoint
	**/
	inline void update() { updatePolynomials(); }
	
private:
	/**
	 * @brief Polynomial
	 *
	 * Representation of a 3rd degree polynomial by 4 coefficients.
	 * P(t) = P[0] + P[1] * t + P[2] * t^2 + P[3] * t^3
	**/
	class Polynomial : public std::array<T, 4>
	{
	public:
		/// @return Evaluation of the polynomial at t
		inline T operator()(R t) const { return (*this)[0] + t * (*this)[1] + t * t * (*this)[2] + t * t * t * (*this)[3]; }
		/// @return Evaluation of the first derivative of the polynomial at t
		inline T d(R t) const { return (*this)[1] + t * static_cast<R>(2.0) * (*this)[2] + t * t * static_cast<R>(3.0) * (*this)[3]; }
		/// @return Evaluation of the second derivative of the polynomial at t
		inline T d2(R t) const { return static_cast<R>(2.0) * (*this)[2] + static_cast<R>(6.0) * t * (*this)[3]; }
		
		/// @return First derivative of the polynomial (So, also a polynomial)
		inline Polynomial d() const { return {(*this)[1], static_cast<R>(2.0) * (*this)[2], static_cast<R>(3.0) * (*this)[3], R()}; }
		/// @return Second derivative of the polynomial (So, also a polynomial)
		inline Polynomial d2() const { return {static_cast<R>(2.0) * (*this)[2], static_cast<R>(6.0) * (*this)[3], R(), R()}; }
	};
	
	std::vector<ControlPoint>	_points;			///< ControlPoints
	std::vector<Polynomial>		_polynomials;		///< Polynomials corresponding to each portion of the spline
	
	/**
	 * @param t Time
	 * @return Index of the ControlPoint right before t
	**/
	size_t getPoint(R t) const;
	
	/**
	 * @brief Compute coefficients for the i-th polynomial.
	**/
	void updatePolynomial(size_t i);
	
	/**
	 * @brief Compute coefficients for the all polynomials.
	**/
	inline void updatePolynomials()
	{
		for(size_t i = 0; i < getPointCount(); ++i)
			updatePolynomial(i);
	}
};

// Implementation Details
#include "CubicSpline.tcc"
