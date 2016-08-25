#pragma once

#include <array>
#include <deque>
#include <iostream>
#include <sstream>
#include <chrono>
#include <functional>

namespace Log
{
constexpr size_t BufferSize = 100;

enum LogType
{
	Info		= 0,
	Warning		= 1,
	Error		= 2
};
extern std::array<const char*, 3>	_log_types;

struct LogLine
{
	std::time_t	time;
	LogType		type;
	std::string	message;
	std::string cached_full;
	
	inline operator std::string()
	{
		return str();
	}
	
	inline std::string str()
	{
		if(cached_full.empty())
		{
			char mbstr[100];
			std::strftime(mbstr, sizeof(mbstr), "%H:%M:%S", std::localtime(&time));
			cached_full = "[" + std::string(mbstr) + "] " + _log_types[type] + ": " + message;
		}
		return cached_full;
	}
};

extern std::ostringstream						_log_line;
extern std::deque<LogLine>						_logs;
extern std::function<void(const LogLine& ll)>	_log_callback;

void _log(LogType lt);

template<typename T, typename ...Args>
inline void _log(LogType lt, const T& msg, Args... args)
{
	_log_line << msg;
	_log(lt, args...);
}

template<typename ...Args>
inline void info(Args... args)
{
	_log(LogType::Info, args...);
}

template<typename ...Args>
inline void warn(Args... args)
{
	_log(LogType::Warning, args...);
}

template<typename ...Args>
inline void error(Args... args)
{
	_log(LogType::Error, args...);
}

};
