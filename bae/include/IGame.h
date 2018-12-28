#pragma once

namespace bae {
class IGame {
public:
    virtual ~IGame(){};

    virtual void start() = 0;
    virtual bool update() = 0;
};
}
