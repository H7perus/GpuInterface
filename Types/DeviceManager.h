#include "Device.h"
#include "Swapchain.h"


#include "GpuInterface/GpuInterface.h"

#include "GpuInterfaceDLL.h"

namespace KE
{
class VkGpuInterface;
}

namespace KE::VK
{
// Singleton to manage devices.
class GPUI_DLL_API ContextManager
{
    friend class KE::VkGpuInterface;
    ContextManager()                                 = default;
    ContextManager& operator=(const ContextManager&) = delete;
    ContextManager(ContextManager&&)                 = delete;
    ContextManager& operator=(ContextManager&&)      = delete;

  protected:
    vkb::Instance vkboot_inst_;

    std::vector<Device> devices_;

    std::map<u32, Swapchain> swapchains_;

  public:
    static void Init();

    static ContextManager& GetInstance();

    static void SetVkInstance(vkb::Instance inst);

    static vk::Instance GetVkInstance();

    static Device& GetDevice(u32 index);

    static Swapchain& GetSwapchain(u32 index);

    //Important note: we are currently assuming that devices will never be destroyed. This is important for indexing. A more robust solution would need a map.
    template <typename... Args> u32 AddDevice(Args&&... args)
    {
        u32 index = GetInstance().devices_.size();
        GetInstance().devices_.emplace_back(std::forward<Args>(args)..., index);
        GetInstance().devices_[index].InitDescriptorHeaps();
        return index;
    }

    template <typename... Args> 
    static u32 AddSwapchain(Args&&... args)
    {
        GetInstance().swapchains_.emplace(GetInstance().swapchains_.size(), Swapchain(std::forward<Args>(args)...));

        return GetInstance().swapchains_.size() - 1;
    }
};
} // namespace KE::VK
