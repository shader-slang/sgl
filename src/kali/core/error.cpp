#include "error.h"

namespace kali {

Exception::Exception(const char* what)
    : m_what(std::make_shared<std::string>(what))
{
}

} // namespace kali
