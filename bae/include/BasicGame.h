#pragma once
#include "IGame.h"
#include "Renderer.h"
#include "utils/Common.h"

namespace bae {
class BasicGame : IGame {
public:
    BasicGame();
    ~BasicGame() noexcept;

    void start() override;
    bool update() override;

private:
    Renderer renderer;
    uint64_t startOffset = 0;
    float lastTime = 0;
};
}
