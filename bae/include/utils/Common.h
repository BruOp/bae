#pragma once
#include "Geometric.h"
#include <bx/timer.h>

namespace bae {
inline float getTime(const uint64_t startOffset)
{
    return static_cast<float>((bx::getHPCounter() - startOffset) / static_cast<double>(bx::getHPFrequency()));
}
} // namespace bae
