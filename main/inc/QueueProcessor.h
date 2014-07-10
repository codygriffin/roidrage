//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_QUEUEPROCESSOR_H
#define INCLUDED_QUEUEPROCESSOR_H

#ifndef INCLUDED_WORKQUEUE_H
#include "WorkQueue.h"
#endif

#ifndef INCLUDED_LOG_H
#include "Log.h"
#endif

#ifndef INCLUDED_THREAD
#include <thread>
#define INCLUDED_THREAD
#endif

//------------------------------------------------------------------------------

namespace corvid { 

//------------------------------------------------------------------------------

struct QueueProcessor {
  QueueProcessor();
  virtual ~QueueProcessor();

  void enqueue(WorkQueue::Work work);

private:
  void loop (); 

  bool                    running_;
  WorkQueue               queue_;
  std::thread             queueThread_;
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#endif
