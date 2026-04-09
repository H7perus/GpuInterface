#include "GpuInterface/Types/GpuTypeRegister.h"

#include "Util/FNV1a.h"
#include <format>
#include <stdexcept>

u32 KE::GpuTypeRegister::incrementingID = 0;

std::unordered_map<u64, KE::GpuTypeInfo, KE::GpuIdentityHash>& KE::GpuTypeRegister::GetTypes()
{
    static std::unordered_map<u64, KE::GpuTypeInfo, KE::GpuIdentityHash> types;
    return types;
}


KE::GpuTypeInfo* KE::GpuTypeRegister::RegisterType(const char* typeName)
{
    auto types = GetTypes();
    u64 hashedName = FNV1aTo64(typeName);

    if (!types.contains(hashedName))
    {
        KE::GpuTypeInfo info{hashedName, typeName};
        types.insert(std::make_pair(hashedName, info));
    }
    else
    {
        throw std::logic_error(
            std::format("Type already exists or colliding hash! \n Existing name: {}  \n Trying to add: {} ",
                        types.at(hashedName).typeName, typeName));
    }

    return &types.at(hashedName);
}