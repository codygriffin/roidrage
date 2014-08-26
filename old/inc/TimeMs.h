//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_TIMEMS_H
#define INCLUDED_TIMEMS_H

#ifndef INCLUDED_TIME_H
#include <time.h>
#define INCLUDED_TIME_H
#endif

#include <chrono>
#include <stdint.h>

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

// TODO This shit sucks 
struct TimeMs {
  TimeMs(int64_t mark = 0)
    : mark_ (mark) {
  }

  static inline int64_t now() {
    #ifdef ANDROID
    timespec n;
    clock_gettime(CLOCK_MONOTONIC, &n);
    return (n.tv_sec*1000) + (n.tv_nsec/1000000);
    #else
    auto current = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(current.time_since_epoch()).count();
    #endif
  }

  inline void markRelative(int64_t when) {
    markAbsolute(TimeMs::now() + when);
  }

  inline void markAbsolute(int64_t when) {
    mark_ = when;
  }

  inline int64_t mark() {
    int64_t t = mark_;
    mark_ = TimeMs::now();
    return mark_ - t;
  }

  inline int64_t since() const {
    return TimeMs::now() - mark_; 
  };

  inline int64_t until() const {
    return mark_ - TimeMs::now(); 
  };

  inline bool expired() const {
    return since() >= 0; 
  };

  inline bool expired(int64_t& when) const {
    when = since();  
    return when >= 0;
  };

  inline bool pending() const {
    return until() > 0 ; 
  };

  inline bool pending(int64_t& when) const {
    when = until();  
    return when > 0;
  };

private:
  int64_t mark_;
};

//------------------------------------------------------------------------------

} // namespace

//------------------------------------------------------------------------------

#endif
