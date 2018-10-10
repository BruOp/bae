#pragma once

#include <array>

namespace bae
{
template <enum Event, size_t bufferSize>
class EventQueue
{
  public:
    void registerEvent(Event event)
    {
        m_queue[m_end] = event;
    };

    void reset()
    {
        m_end = 0;
    };

    std::array<Event, bufferSize>::const_iterator begin() const
    {
        return m_queue.begin();
    };

    std::array<Event, bufferSize>::const_iterator end() const
    {
        return m_queue.begin() + m_end;
    };

    size_t size() const
    {
        return m_end;
    }

    bool isFull() const
    {
        return m_end == bufferSize - 1;
    }

    bool isEmpty() const
    {
        return m_end == 0;
    }

  private:
    std::array<Event, bufferSize> m_queue;
    size_t m_end = 0;
};
} // namespace bae
