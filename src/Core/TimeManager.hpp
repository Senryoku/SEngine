#pragma once

#include <functional>
#include <vector>

#include <Clock.hpp>

namespace TimeManager
{

using Real = double;
using TimePoint = Clock::time_point;
using Seconds = std::chrono::duration<Real>;
/**
 * Parameters: Runtime, DeltaTime
**/
using Callback = std::function<void(Real, Real)>;

// Hiding details behind a namespace
namespace impl
{

constexpr Real MinTimeFrame = 1.f/10000.f;
constexpr Real MaxTimeFrame = 1.f/1.f;

extern Real			_runtime;
	
extern Real			_timerate; 				///< TimeRate
	
extern Real			_cachedRealDeltaTime;	///< Last 'real' frame time in seconds (floating-point) 
extern Real			_cachedDeltaTime;		///< Last in-game frame time in seconds (floating-point)

extern TimePoint	_lastUpdate;			///< TimePoint of the last call to update()

extern std::vector<Callback>	_callbacks;	///< Functions to call on update.

}

inline TimePoint now() { return Clock::now(); }

/** @brief Manages the internals timers - should be called once each frame.
 *  This is where the actual job is done !
 */
void update();

/** @brief Returns the current Timerate
 * (1.f means normal game speed, always >= 0.f)
 */
inline Real getTimerate() { return (impl::_timerate > 0.f) ? impl::_timerate : 0.f; }

/** @brief Sets the timerate to value
 *  @param value
 */
inline void setTimerate(Real value) { if(value >= 0.f) impl::_timerate = value; }

/** @brief Returns the last frame 'real' time duration in seconds
 * This is clamped to avoid 0 cancelation at really high FPS
 * and too long frames (when moving the window for example).
 */
inline Real getRealDeltaTime() { return impl::_cachedRealDeltaTime; }

/** @brief Returns the last frame in-game duration in seconds
 * (equivalent to getTimerate()*getRealDeltaTime())
 */
inline Real getDeltaTime() { return impl::_cachedDeltaTime; }

/** @brief Returns frame rate based on the last frame duration
 */
inline Real getInstantFrameRate() { return 1.f/getRealDeltaTime(); }

inline Real getRuntime() { return Seconds(now() - TimePoint()).count(); }

inline Real getRuntimeAtFrameStart() { return impl::_runtime; }	

inline void subscribe(const Callback& c) { impl::_callbacks.push_back(c); }

}
