#pragma once

#include <iostream>

#ifdef NDEBUG
	#define DEBUG_ASSERT(condition, message)
#else
	#define DEBUG_ASSERT(condition, message) if(!(condition)) { std::cout << message << std::endl; throw; }
#endif