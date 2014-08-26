//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------


#ifndef INCLUDED_RESOURCEMANAGER_H
#define INCLUDED_RESOURCEMANAGER_H

#ifndef INCLUDED_MAP
#include <map>
#define INCLUDED_MAP
#endif

#ifndef INCLUDED_TUPLE
#include <tuple>
#define INCLUDED_TUPLE
#endif

#ifndef INCLUDED_TUPLES_H
#include "Tuples.h"
#endif


//------------------------------------------------------------------------------

namespace beta { 

//------------------------------------------------------------------------------

template <typename...T>
struct ResourceManager {

  template <typename R>
  R* get(const std::string& handle) {
    auto resource = resources<R>().find(handle);
    if (resource != resources<R>().end()) {
      return resource;
    } else {
      // Asset manager smart enough to load different resources
    }
  }

  // TODO I should probably free all of these at some point...

private:
  template <typename R>
  std::map<std::string, R*>& 
  resources() {
    return tuples::getType<
      std::map<std::string, R*>
    >(resources_);
  }

  std::tuple<
    std::map<std::string, T*>...
  > resources_;
};

//------------------------------------------------------------------------------

} // namespace beta

//------------------------------------------------------------------------------

#endif
