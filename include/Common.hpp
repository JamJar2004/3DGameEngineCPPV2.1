#pragma once

#ifdef NDEBUG
	#define DEBUG_ASSERT(condition, message)
#else
	#include <iostream>
	#define DEBUG_ASSERT(condition, message) if(!(condition)) { std::cout << message << std::endl; throw; }
#endif

template<typename TCopyable>
concept ShallowCopyable = std::is_trivially_copyable_v<TCopyable> && std::is_standard_layout_v<TCopyable>;