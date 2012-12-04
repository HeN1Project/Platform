#include "Platform/Condition.h"
#include "Platform/Platform.h"

#include "Platform/Assert.h"

#include <pthread.h>

using namespace Helium;

// Manual-Reset event implementation for pthreads:
// http://www.cs.wustl.edu/~schmidt/win32-cv-2.html

void event_init(Condition::Handle* evt, bool manual_reset, bool initial_state)
{
    evt->manual_reset = manual_reset;
    evt->is_signaled = initial_state;
    evt->waiting_threads = 0;

    pthread_cond_init (&evt->condition, NULL);
    pthread_mutex_init (&evt->lock, NULL);
}

void event_destroy(Condition::Handle* evt)
{
    pthread_mutex_destroy (&evt->lock);
    pthread_cond_destroy (&evt->condition);
}

void event_wait(Condition::Handle* evt)
{
    // grab the lock first
    pthread_mutex_lock (&evt->lock);

    // Condition is currently signaled
    if (evt->is_signaled)
    {
        if (!evt->manual_reset)
        {
            // AUTO: reset state
            evt->is_signaled = 0;
        }
    }
    else // evt is currently not signaled
    {
        evt->waiting_threads++;

        pthread_cond_wait (&evt->condition, &evt->lock);

        evt->waiting_threads--;
    }

    // Now we can let go of the lock
    pthread_mutex_unlock (&evt->lock);
}

void event_signal(Condition::Handle* evt)
{
    // grab the lock first
    pthread_mutex_lock (&evt->lock);

    // Manual-reset evt
    if (evt->manual_reset)
    {
        // signal evt
        evt->is_signaled = true;

        // wakeup all
        pthread_cond_broadcast (&evt->condition);
    }
    // Auto-reset evt
    else
    {
        if (evt->waiting_threads == 0)
        {
            // No waiters: signal evt
            evt->is_signaled = true;
        }
        else
        {
            // Waiters: wakeup one waiter
            pthread_cond_signal (&evt->condition);
        }
    }

    // Now we can let go of the lock
    pthread_mutex_unlock (&evt->lock);
}

void event_pulse(Condition::Handle* evt)
{
    // grab the lock first
    pthread_mutex_lock (&evt->lock);

    // Manual-reset evt
    if (evt->manual_reset)
    {
        // Wakeup all waiters
        pthread_cond_broadcast (&evt->condition);
    }

    // Auto-reset evt: wakeup one waiter
    else
    {
        pthread_cond_signal (&evt->condition);

        // Reset evt
        evt->is_signaled = false;
    }

    // Now we can let go of the lock
    pthread_mutex_unlock (&evt->lock);
}

void event_reset(Condition::Handle* evt)
{
    // Grab the lock first
    pthread_mutex_lock (&evt->lock);

    // Reset evt
    evt->is_signaled = false;

    // Now we can let go of the lock
    pthread_mutex_unlock (&evt->lock);
}

Condition::Condition()
{
    event_init(&m_Handle, true, false);
}

Condition::~Condition()
{
    event_destroy(&m_Handle);
}

void Condition::Signal()
{
    event_signal(&m_Handle);
}

void Condition::Reset()
{
    event_reset(&m_Handle);
}

bool Condition::Wait(u32 timeout)
{
    HELIUM_ASSERT( timeout == 0xffffffff ); // not supported
    event_wait(&m_Handle);
    return true;
}
