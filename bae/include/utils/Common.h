#pragma once
#include "Geometric.h"
#include <bx/timer.h>

namespace bae {
inline float getTime(const uint64_t startOffset)
{
    return (float)((bx::getHPCounter() - startOffset) / double(bx::getHPFrequency()));
}
} // namespace bae
