// SPDX-License-Identifier: Apache-2.0

#include <slang.h>
#include <slang-com-ptr.h>

namespace sgl {

/// Implementation of slang's ISlangBlob interface to access an unowned blob of data.
class UnownedSlangBlob : public ISlangBlob {
public:
    UnownedSlangBlob(const void* data, size_t size)
        : m_data(data)
        , m_size(size)
    {
    }

    virtual SLANG_NO_THROW void const* SLANG_MCALL getBufferPointer() override { return m_data; }
    virtual SLANG_NO_THROW size_t SLANG_MCALL getBufferSize() override { return m_size; }

    virtual SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == SLANG_UUID_ISlangBlob) {
            *outObject = static_cast<ISlangBlob*>(this);
            return SLANG_OK;
        }
        return SLANG_E_NO_INTERFACE;
    }

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override
    {
        // Do not perform any reference counting.
        return 2;
    }

    virtual SLANG_NO_THROW uint32_t SLANG_MCALL release() override
    {
        // Do not perform any reference counting.
        return 2;
    }

private:
    const void* m_data;
    size_t m_size;
};

} // namespace sgl
