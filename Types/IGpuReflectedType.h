#pragma once
#include "GpuInterface/BasicTypeAliases.h"
#include <string>

#include "GpuInterface/Types/GpuTypeInfo.h"
#include "GpuInterface/Types/GpuTypeRegister.h"


namespace KE::VK
{

class IGpuReflectedType
{
  private:
    inline static KE::GpuTypeInfo const *const m_TypeInfo = nullptr;

  public:
    IGpuReflectedType()                  = default;
    virtual ~IGpuReflectedType()         = default;
    virtual u32         GetTypeID()   = 0;                                                                                                                 \
    virtual std::string GetTypeName() = 0;
};

} // namespace KE

#define KE_GPUREFLECT(TypeName)                                                                                           \
    inline static KE::GpuTypeInfo const *const m_TypeInfo = KE::GpuTypeRegister::RegisterType(#TypeName);                          \
                                                                                                                       \
  public:                                                                                                              \
    virtual u32 GetTypeID() override                                                                                   \
    {                                                                                                                  \
        return m_TypeInfo->typeID;                                                                                     \
    }                                                                                                                  \
    static u32 GetStaticTypeID()                                                                                       \
    {                                                                                                                  \
        return m_TypeInfo->typeID;                                                                                     \
    }                                                                                                                  \
    virtual std::string GetTypeName() override                                                                         \
    {                                                                                                                  \
        return #TypeName;                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
  private: