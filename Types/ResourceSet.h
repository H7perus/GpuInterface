#pragma once

#include "Buffer.h"

#include "GpuInterface/BasicTypeAliases.h"
#include "GpuInterface/Types/ResourceHandle.h"
#include <cassert>
#include <stdexcept>
// Purpose: Enable references to flipping buffers and frames-in-flight-aligned buffers (i.e. uniform buffers) in an
// abstract way. Reflection info is then used to find the appropriate set and not a specific buffer.

namespace KE::VK
{
enum ResourceSetUsage
{
    // A single buffer
    STATIC,
    // Two buffers, flip-flopping back and forth ( might not even need logic on the CPU side. We could just decide in
    // shader(?) )
    FLIP,
    // Aligned to Frames-In-Flight. Used for things like uniform buffers.
    FRAMEALIGNED
};

struct ResourceSetDesc
{
    ResourceSetUsage usage             = STATIC;
    u32              numFramesInFlight = 0;
};


class IResourceSet
{
  public:
    virtual ~IResourceSet() = default;

    virtual void SetCurrent(u32 index)  = 0;
    virtual u32  GetSize() const        = 0;
    virtual ResourceHandle GetResourceHandle() = 0;
};


template <std::derived_from<IResource> T>
class ResourceSet final : public IResourceSet
{
    std::vector<std::shared_ptr<IResource>> resources_;
    // This is a constant memory address, meaning that we can just point a vk::DescriptorWriteSet at it
    std::shared_ptr<std::shared_ptr<IResource>> current_ = std::make_shared<std::shared_ptr<IResource>>();

    ResourceSetUsage usage_      = STATIC;
    u8               curr_index_ = 0;

  public:

    ResourceSet() = default;

    ResourceSet(ResourceSetDesc desc, auto&&... args)
    {

        usage_ = desc.usage;
        if (desc.usage == STATIC)
        {
            resources_.push_back(std::make_shared<T>(std::forward<decltype(args)>(args)...));
            *current_ = resources_[0];
        }
        else if (desc.usage == FLIP)
        {
            resources_.push_back(std::make_shared<T>(std::forward<decltype(args)>(args)...));
            resources_.push_back(std::make_shared<T>(std::forward<decltype(args)>(args)...));
            *current_ = resources_[0];
        }
        else if (desc.usage == FRAMEALIGNED)
        {
            assert(!(desc.numFramesInFlight == 0) && "numFramesInFlight must be greater than 0 for FRAMEALIGNED usage");

            for (u32 i = 0; i < desc.numFramesInFlight; i++)
            {
                resources_.push_back(std::make_shared<T>(std::forward<decltype(args)>(args)...));
            }
            *current_ = resources_[0];
        }
    };

    void SetCurrent(u32 index)
    {
        curr_index_ = index;
        *current_   = resources_[index];
    }

    void SetNthResource(u32 index, T resource)
    {
        assert(resources_[0]->GetTypeID() == T::GetStaticTypeID() &&
               "Resource type mismatch in ResourceSet::SetNthResource");
        if (index >= resources_.size())
        {
            throw std::out_of_range("Index out of range in ResourceSet::SetNthResource");
        }
        resources_[index] = std::make_shared<T>(std::move(resource));
    }

    u32 GetSize() const
    {
        return resources_.size();
    }

    ResourceHandle GetResourceHandle()
    {
        return ResourceHandle(std::shared_ptr<std::shared_ptr<IResource>>(current_));
    }
};
} // namespace KE::VK
