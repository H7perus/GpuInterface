#pragma once
#include "GpuInterface/BasicTypeAliases.h"
#include <string>

namespace KE {

	struct GpuTypeInfo
	{
		u64 typeID = 0;
		std::string typeName;
	};
}