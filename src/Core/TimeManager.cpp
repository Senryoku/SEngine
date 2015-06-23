#include <TimeManager.hpp>

TimeManager::TimeManager() :
	_timerate(1.f),
	_cachedRealDeltaTime(s_minTimeFrame),
	_cachedDeltaTime(_timerate*s_minTimeFrame),
	_lastUpdate(now())
{
}

void TimeManager::update()
{   
	TimePoint tmp = now();
    _cachedRealDeltaTime = Seconds(tmp - _lastUpdate).count();
	_lastUpdate = tmp;
	
    // Clamping
    if(_cachedRealDeltaTime < s_minTimeFrame)
    {
		// FPS are way too high ! Assuming a minimum frame time...
        _cachedRealDeltaTime = s_minTimeFrame;
    } else if(_cachedRealDeltaTime > s_maxTimeFrame) {
        // FPS are too low, we can't stand this !
        _cachedRealDeltaTime = s_maxTimeFrame;
    }
	
	_cachedDeltaTime = getTimerate()*getRealDeltaTime();
	_runtime += _cachedDeltaTime;
	_floatRunTime = static_cast<float>(_runtime);
}
