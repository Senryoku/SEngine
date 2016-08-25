#include <Log.hpp>

namespace Log
{

std::array<const char*, 3>	_log_types = {
	"Info",
	"Warning",
	"Error"
};

std::ostringstream						_log_line;
std::deque<LogLine>						_logs;
std::function<void(const LogLine& ll)>	_log_callback;

void _addLogLine(const LogLine& ll)
{
}

void _log(LogType lt)
{
	std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	if(_logs.size() > BufferSize)
		_logs.pop_front();
	_logs.push_back(LogLine{
		now,
		lt,
		_log_line.str()
	});
	if(_log_callback)
		_log_callback(_logs.back());
	_log_line.str("");
}

};
