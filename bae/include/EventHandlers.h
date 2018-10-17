#pragma once
#include "Camera.h"
#include "EventQueue.h"

namespace bae
{

enum EventHandleResult
{
    EVENT_RESULT_SHUTDOWN,
    EVENT_RESULT_CONTINUE
};

class EventSubscriber
{
  public:
    virtual EventHandleResult handleEvents(const EventQueue &eventQueue) = 0;
};

class WindowInputHandler : public EventSubscriber
{
  public:
    EventHandleResult handleEvents(const EventQueue &eventQueue) override final;
};
} // namespace bae
