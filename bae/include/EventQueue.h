#pragma once

#include "SDL.h"
#include <vector>

namespace bae
{
class EventQueue
{
    typedef std::vector<SDL_Event> EventList;
    typedef EventList::iterator iterator;
    typedef EventList::const_iterator const_iterator;

    enum PumpResult
    {
        RESULT_SUCCESS,
        RESULT_EMPTY,
        RESULT_FULL
    };

  public:
    EventQueue() : EventQueue(EventQueue::defaultSize)
    {
    }

    EventQueue(size_t bufferSize)
        : m_bufferSize{bufferSize},
          m_queue(bufferSize)
    {
    }

    void registerEvent(SDL_Event event) noexcept
    {
        m_queue[m_end] = event;
        ++m_end;
    };

    PumpResult pump()
    {
        reset();

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            registerEvent(event);
        }

        if (isEmpty())
        {
            return PumpResult::RESULT_SUCCESS;
        }
        else if (isFull())
        {
            return PumpResult::RESULT_FULL;
        }
        else
        {
            return PumpResult::RESULT_SUCCESS;
        }
    };

    void
    reset()
    {
        m_end = 0;
    };

    // iterator begin()
    // {
    //     return m_queue.begin();
    // };

    // iterator end()
    // {
    //     return m_queue.begin() + m_end;
    // };

    const_iterator begin() const
    {
        return m_queue.cbegin();
    };

    const_iterator end() const
    {
        return m_queue.cbegin() + m_end;
    };

    size_t size() const
    {
        return m_end;
    }

    bool isFull() const
    {
        return m_end == m_bufferSize - 1;
    }

    bool isEmpty() const
    {
        return m_end == 0;
    }

  private:
    size_t m_end = 0;
    size_t m_bufferSize;
    EventList m_queue;

    static constexpr size_t defaultSize = 32;
};
} // namespace bae
