#include <TimeManager.hpp>

namespace TimeManager
{

namespace impl
{

Real					_runtime = 0.f;
Real					_timerate = 1.0f;
Real					_cachedRealDeltaTime = MinTimeFrame;
Real					_cachedDeltaTime = _timerate * MinTimeFrame;

TimePoint				_lastUpdate = now();

std::vector<Callback>	_callbacks;

}

void update()
{   
	using namespace impl;
	TimePoint tmp = now();
    _cachedRealDeltaTime = Seconds(tmp - _lastUpdate).count();
	_lastUpdate = tmp;
	
    // Clamping
    if(_cachedRealDeltaTime < MinTimeFrame)
    {
		// FPS are way too high ! Assuming a minimum frame time...
        _cachedRealDeltaTime = MinTimeFrame;
    } else if(_cachedRealDeltaTime > MaxTimeFrame) {
        // FPS are too low, we can't stand this !
        _cachedRealDeltaTime = MaxTimeFrame;
    }
	
	_cachedDeltaTime = getTimerate() * getRealDeltaTime();
	_runtime += _cachedDeltaTime;
	
	for(auto& f : _callbacks)
		f(_runtime, _cachedRealDeltaTime);
}

}