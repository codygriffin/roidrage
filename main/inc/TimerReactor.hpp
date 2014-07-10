//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_TIMERREACTOR_HPP
#define INCLUDED_TIMERREACTOR_HPP

//------------------------------------------------------------------------------

namespace corvid {

//------------------------------------------------------------------------------

template <typename Duration>
const TimerReactor::Timer& 
TimerReactor::setTimer(Duration duration, Callback callback) {
  return setTimer(Timer(timerId_, callback, Clock::now() + duration, Resolution(0)));
}

template <typename Duration>
const TimerReactor::Timer& 
TimerReactor::setPeriodic(Duration duration, Callback callback) {
  return setTimer(Timer(timerId_, callback, Clock::now() + duration, duration));
}

template <typename Duration>
const TimerReactor::Timer& 
TimerReactor::setPeriodic(TimePoint timePoint, Duration duration, Callback callback) {
  return setTimer(Timer(timerId_, callback, timePoint, duration));
}

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
