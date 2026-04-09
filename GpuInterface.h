#pragma once


//#include "Base/System/System.h"


#include "GpuInterface/BasicTypeAliases.h"

//------------------------------------------------------------------------------------
// Interface for GpuInterface. said GpuInterfaces should be plug-n-play.
// Note that GpuInterface means an interface to the GPU, so this is an interface(class) for a interface(not class), which is not a mistake.
// There is explicitely no "Renderer" interface or the likes, because the GpuInterface is not just responsible for such workloads.
//------------------------------------------------------------------------------------

namespace KE
{
	class IGpuInterface
	{
	public:

		virtual void Init() = 0;

		virtual u64 GetSDLWindowFlag() = 0;

		//virtual void Draw() = 0;

		//virtual void Compute() = 0;
	};
}