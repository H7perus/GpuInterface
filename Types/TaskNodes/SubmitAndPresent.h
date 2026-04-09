#pragma once


#include "../SwapchainOutput.h"
#include "../../Functions/TransitionBarrier.h"

#include "../TaskNode.h"
#include "GpuInterface/Types/Frame.h"


#include "GpuInterface//GpuInterface.h"

#include "GpuInterfaceDLL.h"

namespace KE::VK
{
    //I believe this was a mistake. I will keep frame end and present as part of the swapchain outside of the graph for now.
    class GPUI_DLL_API SubmitAndPresent : public ITaskNode
    {
        u32 swapchainIndex_;
      public:
        SubmitAndPresent(u32 swapchainIndex);

        ~SubmitAndPresent() = default;

        void Execute(vk::CommandBuffer cmd) override;
    };
}