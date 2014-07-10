//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_GESTURETRACKER_H
#define INCLUDED_GESTURETRACKER_H

#include "glm.hpp"
#include <vector>
#include <array>
#include <algorithm>
#include "Log.h"
#include "TimeMs.h"

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Tap {
  glm::vec2 pos;
};

struct Swipe {
  glm::vec2 pos;
  glm::vec2 vel;
};

struct Drag {
  std::vector<glm::vec2> posSamples;
};

struct DoubleTap {
  glm::vec2 pos;
};

struct LongPress {
  glm::vec2 pos;
};

struct PinchSpread {
  float factor;
};

struct Pivot {
  float factor;
};

//------------------------------------------------------------------------------
/*
 *  Higher level type, represents a touch (and its time and validity).
 * 
 */
struct GestureSample {
  glm::vec2 pos; 
  int64_t   time; // TODO move to std::chrono

  GestureSample operator-(const GestureSample& other) const {
    GestureSample sample;
    sample.pos  = pos  - other.pos; 
    sample.time = time - other.time; 
    return sample;
  }
};

//------------------------------------------------------------------------------
/*
 *  Contiguous chunk of valid GestureSamples in time
 * 
 */
struct GestureSegment {
  // How long the finger has been down? TODO should be a constant samplerate
  int64_t getDuration() const {
    return samples_.back().time - samples_.front().time;
  }
  
  // How far the finger has travelled
  float   getLength() const {
    auto sampleLength = [](float l, const GestureSample& s) {
      return l + glm::length(s.pos); 
    };

    return std::accumulate(dsamples_.begin(), dsamples_.end(), 0.0f, sampleLength); 
  }

  void addSample(const GestureSample& sample) {
    if (!samples_.empty()) {
      dsamples_.push_back(sample - samples_.back());
    }

    samples_.push_back(sample);
  }

  std::vector<GestureSample> samples_;
  std::vector<GestureSample> dsamples_;
private:
};

//------------------------------------------------------------------------------

struct GestureStream  {
  GestureStream() 
  : numFingers_(0) {
  }

  void onTouch(Touch touch) {
    for (int i = 0; i < i < touch.count; i++) {
      // Get the current digit
      int finger = touch.pointers[touch.index];

      // Get the current position (screen space)
      glm::vec2 pos  = glm::vec2(touch.xs[touch.index]-Display::getWidth()/2, 
                                 touch.ys[touch.index]-Display::getHeight()/2);
      // Current time
      int64_t  time  = TimeMs::now();

      GestureSample sample;
      sample.pos     = pos;
      sample.time    = time;

      switch (touch.action) {
      case Touch::down:
        if(!segments_[finger]) segments_[finger].reset(new GestureSegment());
        segments_[finger]->addSample(sample);
        break;
      case Touch::up:
        segments_[finger]->addSample(sample);
        break;
      case Touch::move:
        if(!segments_[finger]) segments_[finger].reset(new GestureSegment());
        segments_[finger]->addSample(sample);
        break;
      }

      onUpdate(finger, touch);
    }
  }
  
  void onUpdate(int finger, Touch touch) {
    using namespace pronghorn;

    Log::info("Segment [pos=(%, %), duration=%, length=% finger=%, i=% of %]", 
              segments_[finger]->samples_.back().pos.x, //TODO vec2::toString()
              segments_[finger]->samples_.back().pos.y,
              segments_[finger]->getDuration(), //TODO GestureSegment::toString
              segments_[finger]->getLength(),
              finger, touch.index, touch.count);
  }

protected:
  void onGesture(Tap gesture) {
  }

  void onGesture(Swipe gesture) {
  }

  void onGesture(Drag gesture) {
  }

  void onGesture(DoubleTap gesture) {
  }

  void onGesture(LongPress gesture) {
  }

  void onGesture(PinchSpread gesture) {
  }

  void onGesture(Pivot gesture) {
  }

private:
  std::array<std::unique_ptr<GestureSegment>, 10> segments_;
  unsigned numFingers_;
};

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
