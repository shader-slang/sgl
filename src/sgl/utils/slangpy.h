// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <vector>
#include <map>

#include "sgl/core/macros.h"
#include "sgl/core/fwd.h"
#include "sgl/core/object.h"
#include "sgl/core/enum.h"
#include "sgl/device/fwd.h"

namespace sgl::slangpy {

enum class AccessType {
    none,
    read,
    write,
    readwrite,
};

SGL_ENUM_INFO(
    AccessType,
    {
        {AccessType::none, "none"},
        {AccessType::read, "read"},
        {AccessType::write, "write"},
        {AccessType::readwrite, "readwrite"},
    }
);
SGL_ENUM_REGISTER(AccessType);

class SGL_API NativeType : public Object {
public:
protected:
};

class SGL_API BoundVariableRuntime : public Object {
public:
protected:
};

class SGL_API BoundCallRuntime : public Object {
public:
    BoundCallRuntime() { }

    BoundCallRuntime(
        std::vector<ref<BoundVariableRuntime>>& args,
        std::map<std::string, ref<BoundVariableRuntime>>& kwargs
    )
    {
        m_args = args;
        m_kwargs = kwargs;
    }

protected:
    std::vector<ref<BoundVariableRuntime>> m_args;
    std::map<std::string, ref<BoundVariableRuntime>> m_kwargs;
};


} // namespace sgl::slangpy
