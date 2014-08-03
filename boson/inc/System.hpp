//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_SYSTEM_HPP
#define INCLUDED_SYSTEM_HPP

#include "Index.h"
#include "Component.h"
#include "Entity.h"

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

template <typename...Args>
Index<Args...>& 
System::index() {
  auto item = indices_.find(std::type_index(typeid(Index<Args...>)));
  if (item != indices_.end()) {
    return *static_cast<Index<Args...>*>(item->second);
  }

  throw std::runtime_error("Cannot get non-existent index");
}

template <typename...Args>
void 
System::registerIndex(void (*pVisitor)(Args*...)) {
  registerIndex<Args...>();
}

template <typename T, typename...Args>
void 
System::registerIndex(void (T::*pVisitor)(Args*...)) {
  registerIndex<Args...>();
}

template <typename...Args>
void 
System::registerIndex() {
  auto item = indices_.find(std::type_index(typeid(Index<Args...>)));
  if (item == indices_.end()) {
    indices_.emplace(std::make_pair(std::type_index(typeid(Index<Args...>)), new Index<Args...>()));
  }
}

template <typename...Args>
void 
System::exec(void (*pVisitor)(Args*...)) {
  index<Args...>().foreach(pVisitor);
}

template <typename T, typename...Args>
void 
System::exec(T& t, void (T::*pVisitor)(Args*...)) {
  index<Args...>().foreach(t, pVisitor);
}

template <typename E>
struct Triggerer {
  E& event;
  Triggerer(E& e) : event(e) {}

  void call(Handler<E>* e) {
    e->handler(event);
  }
};


template <typename E>
void 
System::registerEvent() {
  auto item = indices_.find(std::type_index(typeid(Index<Handler<E>>)));
  if (item == indices_.end()) {
    indices_.emplace(std::make_pair(std::type_index(typeid(Index<Handler<E>>)), new Index<Handler<E>>()));
  }
}

template <typename E>
void 
System::trigger(E event) {
  auto t = Triggerer<E>(event);
  index<Handler<E>>().foreach(t, &Triggerer<E>::call);
}

template <typename V> 
void
System::visit(V v) {
  for (auto kv : entities_) { 
    v(kv.second);
  }
}

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
