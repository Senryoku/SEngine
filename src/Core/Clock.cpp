#include <Clock.hpp>

#if defined (__WIN32__)
bool Clock::is_inited = false;
LARGE_INTEGER Clock::frequency;
#endif
