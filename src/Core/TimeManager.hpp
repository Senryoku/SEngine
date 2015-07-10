#pragma once

#include <Clock.hpp>

#include <Singleton.hpp>

class TimeManager : public Singleton<TimeManager>
{
	public:
	using Real = double;
	using TimePoint = Clock::time_point;
	using Seconds = std::chrono::duration<Real>;
	
	/** @brief Default Constructor
	 *  
	 */
	TimeManager();
	
    /** @brief Manages the internals timers - should be called once each frame.
     *  This is where the actual job is done !
     */
    void update();
	
    /** @brief Returns the current Timerate
     * (1.f means normal game speed, always >= 0.f)
     */
    inline Real getTimerate() const { return (_timerate > 0.f) ? _timerate : 0.f; }
	
    /** @brief Sets the timerate to value
     *  @param value
     */
    inline void setTimerate(Real value) { if(value >= 0.f) _timerate = value; }
	
    /** @brief Returns the last frame 'real' time duration in seconds
     * This is clamped to avoid 0 cancelation at really high FPS
     * and too long frames (when moving the window for example).
     */
    inline Real getRealDeltaTime() const { return _cachedRealDeltaTime; }
	
    /** @brief Returns the last frame in-game duration in seconds
     * (equivalent to getTimerate()*getRealDeltaTime())
     */
    inline Real getDeltaTime() const { return _cachedDeltaTime; }
	
    /** @brief Returns frame rate based on the last frame duration
     */
	inline Real getInstantFrameRate() const { return 1.f/getRealDeltaTime(); }
	
	inline Real getRuntime() const { return _runtime; }
	
	inline Real& getRuntimeRef() { return _floatRunTime; }
	
	private:
	static constexpr Real s_minTimeFrame = 1.f/10000.f;
	static constexpr Real s_maxTimeFrame = 1.f/1.f;
	
	Real		_runtime = 0.f;
	Real		_floatRunTime;
	
	Real		_timerate; 				///< TimeRate
	
	Real		_cachedRealDeltaTime;	///< Last 'real' frame time in seconds (floating-point) 
	Real		_cachedDeltaTime;		///< Last in-game frame time in seconds (floating-point)
	
	TimePoint	_lastUpdate;			///< TimePoint of the last call to update()
	
	inline TimePoint now() const
	{ return Clock::now(); }
};
