//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_INDEX_HPP
#define INCLUDED_INDEX_HPP

#include "Tuples.h"
#include "Entity.h"

namespace boson {

template<typename...Args>
void 
Index<Args...>::index(Entity& entity) {
  if (entity.hasAll<Args...>()) {
    index_.insert(entity.project<Args...>());
  }
}

template <typename...Args>
void 
Index<Args...>::remove(Entity& entity) {
  if (entity.hasAll<Args...>()) {
    auto e = std::find(index_.begin(), index_.end(), entity.project<Args...>());
    if (e != index_.end()) {
      index_.erase(e);
    } 
  }
}

template <typename...Args>
void 
Index<Args...>::foreach(void (*pVisitor)(Args*...)) {
  for (auto c : index_) {
    tuples::apply(pVisitor, c);
  } 
}

template <typename...Args>
template <typename T>
void 
Index<Args...>::foreach(T& context, void (T::*pVisitor)(Args*...)) {
  for (auto c : index_) {
    tuples::apply(context, pVisitor, c);
  } 
}

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
