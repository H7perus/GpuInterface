#include <string>
#include <unordered_map>

#include "GpuInterface/BasicTypeAliases.h"
#include "GpuInterface/Types/GpuTypeInfo.h"

#include "GpuInterfaceDLL.h"

namespace KE
{
	//Purpose: Make sure the unordered map doesn't do extra work. We want to create entities both by string and ID, so it is beneficial to have control over the hashing.
struct GpuIdentityHash
{
    size_t operator()(uint64_t v) const noexcept
    {
        return v;
    }
};


class GPUI_DLL_API GpuTypeRegister
{

  private:
    static std::unordered_map<u64, KE::GpuTypeInfo, KE::GpuIdentityHash> types;

    static u32 incrementingID;

  public:
    static std::unordered_map<u64, KE::GpuTypeInfo, KE::GpuIdentityHash>& GetTypes();

    static KE::GpuTypeInfo* RegisterType(const char* typeName);
};
} // namespace KE