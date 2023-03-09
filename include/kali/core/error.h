#pragma once

#include <stdexcept>

#define KALI_THROW(msg)                                                                                                \
    {                                                                                                                  \
        throw std::runtime_error(msg);                                                                                 \
    }
