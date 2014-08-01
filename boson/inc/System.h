//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SYSTEM_H
#define INCLUDED_SYSTEM_H

#include <string>
#include <typeinfo>
#include <typeindex>
#include <map>

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

struct IIndex;
template <typename...Args> 
struct Index;
struct Entity;

struct System {
  template <typename...Args>
  Index<Args...>&
  index();

  template <typename...Args>
  void 
  registerIndex(void (*pVisitor)(Args*...));

  template <typename T, typename...Args>
  void 
  registerIndex(void (T::*pVisitor)(Args*...));

  template <typename...Args>
  void 
  registerIndex();
  
  template <typename...Args>
  void 
  exec(void (*pVisitor)(Args*...));

  template <typename T, typename...Args>
  void 
  exec(T&, void (T::*pVisitor)(Args*...));

  Entity& 
  entity(const std::string& name);

  void 
  remove(const std::string& name);

  Entity& 
  entity();
  
  void
  indexEntity(Entity& e);

private:
  std::map<std::string,      Entity>   entities_;
  std::map<std::type_index, IIndex*>   indices_;
  std::map<std::type_index, void*>     contexts_;
};

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#include "System.hpp"

//------------------------------------------------------------------------------

#endif
