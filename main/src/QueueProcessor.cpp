//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include <QueueProcessor.h>
#include "Log.h"

//------------------------------------------------------------------------------

using namespace corvid;
using namespace pronghorn;

//------------------------------------------------------------------------------

QueueProcessor::QueueProcessor() 
: running_     (true)
, queueThread_ (&QueueProcessor::loop, this) {
}

//------------------------------------------------------------------------------

QueueProcessor::~QueueProcessor() {
  running_ = false; 
  queue_.kick();
  queueThread_.join();
}

//------------------------------------------------------------------------------

void 
QueueProcessor::enqueue(WorkQueue::Work work) {
  // Can we insertion sort?
  queue_.enqueue(work);
}

//------------------------------------------------------------------------------

void 
QueueProcessor::loop () {
  while (running_) {
    try {
      if (!queue_.work()) {
        queue_.wait();
        continue;
      }
    }
    catch (std::exception& e) {
      Log::error(e.what());
    }
  }
} 

//------------------------------------------------------------------------------
