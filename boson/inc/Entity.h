//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_ENTITY_H
#define INCLUDED_ENTITY_H

#include <stdexcept>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <map>

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

struct System;

struct Entity {
  Entity(System& system, const std::string& name);
 
  void clear();
  
  // Projections are pointers because references don't alias correctly...
  template <typename...T> 
  std::tuple<T*...> project();

  template <typename...T> 
  bool hasAll();

  template <typename T, typename...Args>
  void add(Args...args);

  template <typename T, typename...Args>
  void replace(Args...args);

  void rem(const std::type_index& i);

  template <typename T>
  bool has();

  template <typename T>
  T* get();

  const std::string& name() const { return name_; }

private:
  template <typename T>
  std::type_index 
  static index() { return std::type_index(typeid(T)); }

  std::string name_;
  System&     system_;

  std::map<std::type_index, void*> components_;
};

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#include "Entity.hpp"

//------------------------------------------------------------------------------

#endif
