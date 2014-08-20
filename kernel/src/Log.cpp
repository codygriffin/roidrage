//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#include "Log.h"

//------------------------------------------------------------------------------

using namespace pronghorn;

//------------------------------------------------------------------------------

namespace pronghorn {
  void logCppString(const std::string& string) {
    std::cout << string << std::endl;
  }
  

  std::stack<const char*> Log::tags_;
  unsigned Log::dropped_ = 0;
  float Log::max_ = 25.0f;
  float Log::rateSecs_ = 5.0f;
  float Log::buffer_ = 25.0f;
}

void 
Log::error(const char* msg) {
  std::stringstream stream;
  Log::log(stream, msg);
  logCppString(stream.str());
}

//------------------------------------------------------------------------------

void 
Log::warn(const char* msg) {
  std::stringstream stream;
  Log::log(stream, msg);
  logCppString(stream.str());
}

//------------------------------------------------------------------------------

void 
Log::info(const char* msg) {
  std::stringstream stream;
  Log::log(stream, msg);
  logCppString(stream.str());
}

//------------------------------------------------------------------------------

void 
Log::debug(const char* msg) {
  if (isRateLimited()) {
    dropped_++;
    return;
  }

  if (dropped_ > 0) {
    Log::warn("Log::debug dropped % messages", dropped_);
    dropped_ = 0;
  }

  std::stringstream stream;
  Log::log(stream, msg);
  logCppString(stream.str());
}

//------------------------------------------------------------------------------

void 
Log::log(std::stringstream& stream, const char* msg) {
  while (*msg) {
    if (*msg == '%' && *(++msg) != '%') {
      throw std::runtime_error("invalid format string: missing arguments");
    }
    stream << *msg++;
  }
}

//------------------------------------------------------------------------------

void 
Log::pushTag(const char* tag) {
  Log::tags_.push(tag);
}

//------------------------------------------------------------------------------

void 
Log::popTag() {
  Log::tags_.pop();
}

//------------------------------------------------------------------------------

bool 
Log::isRateLimited() {
  return false;
}

//------------------------------------------------------------------------------

