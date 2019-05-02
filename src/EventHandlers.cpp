#include "EventHandlers.h"

namespace bae
{
EventHandleResult WindowInputHandler::handleEvents(const EventQueue &eventQueue)
{
    for (const auto &event : eventQueue) {
        if (event.type == SDL_QUIT) {
            return EventHandleResult::EVENT_RESULT_SHUTDOWN;
        }
    }
    return EventHandleResult::EVENT_RESULT_CONTINUE;
}
}  // namespace bae
