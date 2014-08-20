//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "TimerReactor.h"
#include "StopWatch.h"
#include "Log.h"
#include <cmath>

//------------------------------------------------------------------------------

using namespace pronghorn;
using namespace corvid;

//------------------------------------------------------------------------------

TimerReactor::Timer::Timer(uint64_t k, TimerReactor::Callback c, TimerReactor::TimePoint t, TimerReactor::Resolution p) 
: key       (k)
, callback  (c)
, timePoint (t) 
, period    (p) {
}

TimerReactor::TimerReactor() 
: running_     (true)
, timerThread_ (&TimerReactor::loop, this) {
  clearStats(true);
}

TimerReactor::~TimerReactor() {
  running_ = false; 
  timerCond_.notify_one();
  timerThread_.join();

  while (!timerList_.empty()) {
    timerList_.pop_front();
    stats.numPendingTimers--;
  }
}

void 
TimerReactor::clearStats(bool clearCounters) { 
  stats.avgTimerDispatchNanos = 0;
  stats.sumTimerDispatchNanos = 0;
  stats.minTimerDispatchNanos = std::numeric_limits<uint64_t>::max();
  stats.maxTimerDispatchNanos = 0;

  stats.avgTimerJitterNanos   = 0;
  stats.sumTimerJitterNanos   = 0;
  stats.minTimerJitterNanos   = std::numeric_limits<uint64_t>::max();
  stats.maxTimerJitterNanos   = 0;

  if (clearCounters) {
    stats.numTimerDispatches    = 0;
    stats.numPendingTimers      = 0;
  }
}

const TimerReactor::Timer& 
TimerReactor::setTimer(TimePoint timePoint, Callback callback) {
  return setTimer(Timer(timerId_, callback, timePoint, Resolution(0)));
}

TimerReactor::Stats 
TimerReactor::getStats() const {
  return stats;
}

const TimerReactor::Timer& 
TimerReactor::setTimer(const Timer& timer) {
  std::unique_lock<std::mutex> lock(timerMutex_);

  // Can we insertion sort?
  timerList_.push_back(timer);
  timerList_.sort([](const Timer& a, const Timer& b) {
    return a.timePoint < b.timePoint;
  });

  // There is a chance that this new timer is the earliest, so we 
  // should notify the reactor to wake up
  timerCond_.notify_one();

  stats.numPendingTimers++;
  return timer;
}

void 
TimerReactor::loop () {
  while (running_) {
    try {
      std::unique_lock<std::mutex> lock(timerMutex_);
      // This could be improved
      if (!timerList_.empty()) {
        timerCond_.wait_until(lock, timerList_.front().timePoint);
      } else {
        std::this_thread::yield();
        continue;
      }
   
      while (!timerList_.empty() && Clock::now() > timerList_.front().timePoint) {
        auto timer = timerList_.front();

        auto start = Clock::now();
        uint64_t dispatchNanos = StopWatch::clock([&]() {
          timer.callback();
        });

        uint64_t jitterNanos   = std::abs(std::chrono::duration_cast<std::chrono::nanoseconds>(start - timer.timePoint).count());

        stats.sumTimerDispatchNanos += dispatchNanos;
        if (dispatchNanos < stats.minTimerDispatchNanos) {
          stats.minTimerDispatchNanos = dispatchNanos;
        } 
        if (dispatchNanos > stats.maxTimerDispatchNanos) {
          stats.maxTimerDispatchNanos = dispatchNanos;
        } 

        stats.sumTimerJitterNanos += jitterNanos;
        if (jitterNanos < stats.minTimerJitterNanos) {
          stats.minTimerJitterNanos = jitterNanos;
        } 
        if (jitterNanos > stats.maxTimerJitterNanos) {
          stats.maxTimerJitterNanos = jitterNanos;
        } 
        
        stats.numTimerDispatches++;
        stats.avgTimerDispatchNanos = stats.sumTimerDispatchNanos / stats.numTimerDispatches;
        stats.avgTimerJitterNanos = stats.sumTimerJitterNanos     / stats.numTimerDispatches;

        timerList_.pop_front();
        stats.numPendingTimers--;

        if (timer.period > Resolution(0)) {
          timer.timePoint = Clock::now() + timer.period;

          // Can we insertion sort?
          timerList_.push_back(timer);
          timerList_.sort([](const Timer& a, const Timer& b) {
            return a.timePoint < b.timePoint;
          });
          stats.numPendingTimers++;
        }
      }
    }
    catch (std::exception& e) {
      Log::error(e.what());
    }
  }
} 

//------------------------------------------------------------------------------
