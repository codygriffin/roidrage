//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_LOG_H
#define INCLUDED_LOG_H

#ifndef INCLUDED_STACK
#include <stack>
#define INCLUDED_STACK
#endif

#ifndef INCLUDED_SSTREAM
#include <sstream>
#define INCLUDED_SSTREAM
#endif

//------------------------------------------------------------------------------

namespace pronghorn {

//------------------------------------------------------------------------------

class Log {
public:
  template<class H, class... T>
  static void error(const char*, H value, T... args);  
  template<class H, class... T>
  static void warn (const char*, H value, T... args);  
  template<class H, class... T>
  static void info (const char*, H value, T... args);  
  template<class H, class... T>
  static void debug(const char*, H value, T... args);  

  static void error(const char*);  
  static void warn (const char*);  
  static void info (const char*);  
  static void debug(const char*);  

  static void pushTag(const char* tag);
  static void popTag();

  static bool isRateLimited();
private:
  template<class H, class... T>
  static void log(std::stringstream&, const char*, H value, T... args);  
  static void log(std::stringstream&, const char*);  

  static std::stack<const char*> tags_;

  static unsigned         dropped_;
  static float            max_;
  static float            rateSecs_;
  static float            buffer_;
};

//------------------------------------------------------------------------------

} // namespaces

//------------------------------------------------------------------------------

#include "Log.hpp"

#endif
