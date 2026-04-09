#include "StageBarrier.h"


KE::VK::StageBarrier::StageBarrier(vk::PipelineStageFlags2 srcStageMask, vk::PipelineStageFlags2 dstStageMask)
{
    barrier_.srcStageMask       = srcStageMask;
    barrier_.srcAccessMask      = vk::AccessFlagBits2::eNone;
    barrier_.dstStageMask       = dstStageMask;
    barrier_.dstAccessMask      = vk::AccessFlagBits2::eNone;
    depInfo_.memoryBarrierCount = 1;
    depInfo_.pMemoryBarriers    = &barrier_;
}

void KE::VK::StageBarrier::Execute(vk::CommandBuffer cmd)
{
    cmd.pipelineBarrier2(depInfo_);
}
