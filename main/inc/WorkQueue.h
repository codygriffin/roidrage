//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_WORKQUEUE_H
#define INCLUDED_WORKQUEUE_H

#include <queue>
#include <condition_variable>
#include <mutex>

//------------------------------------------------------------------------------

namespace corvid { 

//------------------------------------------------------------------------------

struct WorkQueue  {
  typedef std::chrono::high_resolution_clock  Clock;
  typedef std::function<void (void)>  Work;
  typedef std::function<void (void)>  Callback;

  struct Stats {
    uint64_t numPuts;
    uint64_t numGets;
    size_t   numDepth;

    size_t   maxDepth;
  };

  WorkQueue();
  void onHighwater (size_t highwater, Callback callback); 
  void onLowwater (size_t lowwater, Callback callback); 
  void enqueue(Work work); 
  
  void kick(); 
  bool work();   
  void wait();

  Stats getStats() const;

private:
  std::queue<Work>        queue_;
  std::mutex              queueMutex_;
  std::condition_variable queueCond_;

  size_t                  highwater_;
  size_t                  lowwater_;
  Callback                onHighwater_;
  Callback                onLowwater_;

  Stats                   stats_;

};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#endif
