//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_LOG_HPP
#define INCLUDED_LOG_HPP

#ifndef INCLUDED_IOSTREAM
#define INCLUDED_IOSTREAM
#include <iostream>
#endif

#ifndef INCLUDED_STDEXCEPT
#define INCLUDED_STDEXCEPT
#include <stdexcept>
#endif

//------------------------------------------------------------------------------

namespace pronghorn {
  
void logCppString(const std::string& string);
  
//------------------------------------------------------------------------------

template<class H, class... T>
void 
Log::error(const char* msg, H value, T... args) {
  std::stringstream stream;
  Log::log(stream, msg, value, args...);
  logCppString(stream.str());
}

template<class H, class... T>
void 
Log::warn(const char* msg, H value, T... args) {
  std::stringstream stream;
  Log::log(stream, msg, value, args...);
  logCppString(stream.str());
}

template<class H, class... T>
void 
Log::info(const char* msg, H value, T... args) {
  std::stringstream stream;
  Log::log(stream, msg, value, args...);
  logCppString(stream.str());
}

template<class H, class... T>
void 
Log::debug(const char* msg, H value, T... args) {
  if (isRateLimited() && sizeof...(args) > 0) {
    dropped_++;
    return;
  }

  if (dropped_ > 0) {
    Log::warn("Log::debug dropped % messages", dropped_);
    dropped_ = 0;
  }

  std::stringstream stream;
  Log::log(stream, msg, value, args...);
  logCppString(stream.str());
}

template<class H, class... T>
void 
Log::log(std::stringstream& stream, const char* msg, H value, T... args) {
  while (*msg) {
    if (*msg == '%' && *(++msg) != '%') {
      stream << value;
      Log::log(stream, msg, args...); // call even when *s == 0 to detect extra arguments
      return;
    }
    stream << *msg++;
  }
  throw std::logic_error("extra arguments provided to printf");
}

//------------------------------------------------------------------------------

} // namespaces

//------------------------------------------------------------------------------

#endif
