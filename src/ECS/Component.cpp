#include "Component.hpp"

namespace ECS
{
	uint8_t ComponentType::s_lastID(0);

	std::ostream& operator<<(std::ostream& stream, const ComponentType& type)
	{
		stream << type.Name;
		return stream;
	}
}
