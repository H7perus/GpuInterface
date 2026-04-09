#pragma once

#include "../TaskNode.h"

#include "GpuInterface/GpuInterface.h"

#include "GpuInterfaceDLL.h"

namespace KE::VK
{
// This is a node that only contains a barrier. It can be used to synchronise between different renderpasses, for
// example.
class GPUI_DLL_API StageBarrier : public ITaskNode
{
    vk::MemoryBarrier2 barrier_;
    vk::DependencyInfo depInfo_;

  public:
    StageBarrier(vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask);

    ~StageBarrier() = default;

    void Execute(vk::CommandBuffer cmd) override;
};
} // namespace KE::VK
