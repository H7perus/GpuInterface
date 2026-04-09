#pragma once
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "ResourceHandle.h"
#include "vulkan/vulkan.hpp"

// #include "ResourceManager.h"

#include <vulkan/vulkan.hpp>

#include <map>

namespace KE::VK
{
class ITaskNode
{
  friend class TaskGraph;
  protected:
    // inputs and outputs
    std::map<std::string, ResourceHandle> reads_;
    std::map<std::string, ResourceHandle> writes_;

    // for synchronisation. Has to contain the flag bits of all the actions in the node.
    vk::PipelineStageFlags2 stageFlags_;

  public:
    virtual ~ITaskNode() = default;

    void Resolve(u32 deviceIndex)
    {
    }

    virtual void Execute(vk::CommandBuffer cmd) = 0;

  protected:
    template <typename T = IResource> T& GetReadResourceRef(std::string name)
    {
        return *std::static_pointer_cast<T>(reads_[name].GetResourcePtr());
    }

    template <typename T = IResource> shared_ptr<T> GetWriteResourcePtr(std::string name)
    {
        return std::static_pointer_cast<T>(writes_[name].GetResourcePtr());
    }

    void Reads(std::string resourceName)
    {
        reads_.insert({resourceName, ResourceHandle()});
    }
    void Writes(std::string resourceName)
    {
        writes_.insert({resourceName, ResourceHandle()});
    }
};
} // namespace KE::VK