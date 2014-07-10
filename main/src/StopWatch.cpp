//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <StopWatch.h>

//------------------------------------------------------------------------------

using namespace corvid;

//------------------------------------------------------------------------------

uint64_t 
StopWatch::clock(std::function<void (void)> func) {
  auto start = Clock::now();
  func();
  auto end  = Clock::now();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

//------------------------------------------------------------------------------
