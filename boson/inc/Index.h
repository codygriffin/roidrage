//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_INDEX_H
#define INCLUDED_INDEX_H

#include <tuple>
#include <set>

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

struct Entity;
struct IIndex {
  virtual void index(Entity& entity) = 0;
  virtual void remove(Entity& entity) = 0;
};

template<typename...Args>
struct Index : public IIndex {
  typedef std::tuple<Args*...> Key;

  void index(Entity& entity);

  void remove(Entity& entity);

  void 
  foreach(void (*pVisitor)(Args*...));

  template<class T>
  void 
  foreach(T& context, void (T::*pVisitor)(Args*...));

  std::set<Key> index_;
};


//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#include "Index.hpp"

//------------------------------------------------------------------------------

#endif
