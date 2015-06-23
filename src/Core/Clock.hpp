#pragma once

#include <chrono>

#if defined (__WIN32__)

#include <windows.h>
#include <stdexcept>

struct Clock
{
public:
	typedef std::chrono::nanoseconds						duration; // nanoseconds resolution
	typedef duration::rep										rep;
	typedef duration::period									period;
	typedef std::chrono::time_point<Clock, duration>		time_point;
	static bool is_steady; // = true
	
	static time_point now()
	{
		if(!is_inited)
		{
			init();
			is_inited = true;
		}
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		return time_point(duration(static_cast<rep>((double)counter.QuadPart / frequency.QuadPart *
													period::den / period::num)));
	}

private:
	static bool is_inited; // = false
	static LARGE_INTEGER frequency;
	
	static void init()
	{
		if(QueryPerformanceFrequency(&frequency) == 0)
			throw std::logic_error("QueryPerformanceCounter not supported: " + std::to_string(GetLastError()));
	}
};

#else
using Clock = std::chrono::high_resolution_clock;
#endif
