//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_STOPWATCH_H
#define INCLUDED_STOPWATCH_H

#include <chrono>
#include <functional>

//------------------------------------------------------------------------------

namespace corvid { 

//------------------------------------------------------------------------------

struct StopWatch {
  typedef std::chrono::high_resolution_clock  Clock;
  static uint64_t clock(std::function<void (void)> func);
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
