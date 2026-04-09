#pragma once


#include "Resource.h"
#include <cassert>
#include <memory>

#include <stdexcept>
#include <string>
using std::shared_ptr;
namespace KE::VK
{

class ResourceHandle
{
  public:
    ResourceHandle() = default;

    ResourceHandle(shared_ptr<shared_ptr<IResource>> resource) : m_resource(resource)
    {
    }
    bool MatchesType(const ResourceHandle &other) const
    {
        auto resource = m_resource.lock();
        assert(resource && "ResourceSet no longer exists");

        auto resourceOther = other.m_resource.lock();
        assert(resourceOther && "Other ResourceSet no longer exists");

        return (*resource)->GetTypeID() == (*resourceOther)->GetTypeID();
    }

    shared_ptr<IResource> GetResourcePtr() const
    {
        auto resource = m_resource.lock();
        assert(resource && "ResourceSet no longer exists");
        return *resource;
    }

    template <std::derived_from<IResource> T>
    T& GetResourceRef() const
    {
        auto resource = m_resource.lock();
        assert(resource && "ResourceSet no longer exists");
        if((*resource)->GetTypeID() == T::GetStaticTypeID())
        {
            return (*std::static_pointer_cast<T>(*resource));
        }
        else
        {
            throw std::logic_error("Resource type mismatch");
        }
    }

  private:
    std::weak_ptr<shared_ptr<IResource>> m_resource;
};

} // namespace KE::VK
