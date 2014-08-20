//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <WorkQueue.h>
#include <StopWatch.h>

//------------------------------------------------------------------------------

using namespace corvid;

//------------------------------------------------------------------------------

WorkQueue::WorkQueue() {
  stats_.numPuts  = 0;
  stats_.numGets  = 0;
  stats_.numDepth = 0;
  stats_.maxDepth = 0;
  highwater_   = 0;
  lowwater_    = 0;
  onHighwater_ = []() {};
  onLowwater_  = []() {};
}

//------------------------------------------------------------------------------

void 
WorkQueue::onHighwater (size_t highwater, Callback callback) {
  highwater_   = highwater;
  onHighwater_ = callback;
}

//------------------------------------------------------------------------------

void 
WorkQueue::onLowwater (size_t lowwater, Callback callback) {
  lowwater_   = lowwater;
  onLowwater_ = callback;
}

//------------------------------------------------------------------------------

void 
WorkQueue::enqueue(Work work) {
  std::unique_lock<std::mutex> lock(queueMutex_);

  // Can we insertion sort?
  queue_.push(work);
  stats_.numDepth++;
  stats_.numPuts++;
  if (stats_.numDepth > stats_.maxDepth) {  
    stats_.maxDepth = stats_.numDepth;
  }

  // Check our highwater mark
  if (queue_.size() == highwater_) {
    onHighwater_();
  }

  // If we have anyone waiting, let them know
  // they have shit to do
  kick();
}

//------------------------------------------------------------------------------

void 
WorkQueue::kick() {
  queueCond_.notify_one();
}

//------------------------------------------------------------------------------

bool 
WorkQueue::work() {
  std::unique_lock<std::mutex> lock(queueMutex_);
  // If there is work to be done
  if (!queue_.empty()) {
    // Do some work!
    uint64_t time = StopWatch::clock([&]() {
      queue_.front()();
    });

    queue_.pop();

    // Check our lowwater mark
    if (queue_.size() == lowwater_) {
      onLowwater_();
    }

    stats_.numDepth--;
    stats_.numGets++;
    return true;
  }   

  // We did no work...
  return false;
}

//------------------------------------------------------------------------------

void 
WorkQueue::wait() {
   std::unique_lock<std::mutex> lock(queueMutex_);
   // This will wait for the queue to be kicked
   // The queue will be kicked when work is enqueued, 
   // or when the queue is destructed
   queueCond_.wait(lock);
}

//------------------------------------------------------------------------------

WorkQueue::Stats 
WorkQueue::getStats() const {
  return stats_;
}

//------------------------------------------------------------------------------
