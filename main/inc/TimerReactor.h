//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_TIMERREACTOR_H
#define INCLUDED_TIMERREACTOR_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <list>
#include <chrono>
#include <functional>

//------------------------------------------------------------------------------

namespace corvid { 

//------------------------------------------------------------------------------

struct TimerReactor {
  // TODO: Parameterize?
  typedef std::chrono::high_resolution_clock  Clock;
  typedef Clock::time_point                   TimePoint;
  typedef std::chrono::nanoseconds            Resolution;
  typedef std::function<void (void)>          Callback;

  struct Stats {
    uint64_t avgTimerDispatchNanos;
    uint64_t sumTimerDispatchNanos;
    uint64_t minTimerDispatchNanos;
    uint64_t maxTimerDispatchNanos;

    uint64_t avgTimerJitterNanos;
    uint64_t sumTimerJitterNanos;
    uint64_t minTimerJitterNanos;
    uint64_t maxTimerJitterNanos;

    uint64_t numTimerDispatches;
    uint64_t numPendingTimers;
  };

  struct Timer {
    uint64_t   key;
    Callback   callback;
    TimePoint  timePoint;
    Resolution period;
    Timer(uint64_t k, Callback c, TimePoint t, Resolution p);
  };

  TimerReactor();
  virtual ~TimerReactor();

  void clearStats(bool clearCounters = false);

  template <typename Duration>
  const Timer& setTimer(Duration duration, Callback callback);

  template <typename Duration>
  const Timer& setPeriodic(Duration duration, Callback callback);

  const Timer& setTimer(TimePoint timePoint, Callback callback);

  template <typename Duration>
  const Timer& setPeriodic(TimePoint timePoint, Duration duration, Callback callback);

  Stats getStats() const;

protected:

private:
  const Timer& setTimer(const Timer& timer);
  void loop ();


  bool                    running_;

  std::list<Timer>        timerList_;
  std::thread             timerThread_;
  std::mutex              timerMutex_;
  std::condition_variable timerCond_;

  uint64_t                timerId_;
  Stats                   stats;
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#include "TimerReactor.hpp"

//------------------------------------------------------------------------------

#endif
