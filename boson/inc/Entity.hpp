//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_ENTITY_HPP
#define INCLUDED_ENTITY_HPP

#include "Component.h"
#include "System.h"

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

template <typename...T>
struct EntityProj;

template <typename Head, typename...Tail>
struct EntityProj<Head, Tail...> {
  static std::tuple<Head*, Tail*...>
  project(Entity& entity) {
    auto component = entity.get<Head>();
    auto head = std::tuple<Head*>(component);
    auto tail = EntityProj<Tail...>::project(entity);
    return std::tuple_cat(head, tail);
  }

  static bool
  hasAll(Entity& entity) {
    if (entity.has<Head>()) {
      return EntityProj<Tail...>::hasAll(entity);
    } else {
      return false;
    }
  }
};

template <>
struct EntityProj<> {
  static std::tuple<>
  project(Entity& entity) {
    return std::tuple<>();
  }
  
  static bool
  hasAll(Entity& entity) { 
    return true;
  }
};

}

//------------------------------------------------------------------------------


namespace boson {

template <typename...T> 
std::tuple<T*...> 
Entity::project() {
  return EntityProj<T...>::template project(*this);
}

template <typename...T> 
bool
Entity::hasAll() {
  return EntityProj<T...>::template hasAll(*this);
}

template <typename T, typename...Args>
void
Entity::add(Args...args) {
  auto item = components_.find(index<T>());
  if (item == components_.end()) {
    components_.emplace(std::make_pair(index<T>(), new T(args...)));
    system_.indexEntity(*this);
    return;
  }

  throw std::runtime_error("Cannot add duplicate component");
}

template <typename T, typename...Args>
void
Entity::replace(Args...args) {
  auto item = components_.find(index<T>());
  if (item != components_.end()) {
    // No allocation - just destruct and placement-new
    static_cast<T*>(item->second)->~T();
    new (item->second) T(args...);
    return;
  }

  throw std::runtime_error("Cannot replace non-existent component");
}

template <typename T, typename...Args>
void
Entity::addOrReplace(Args...args) {
  if (has<T>()) {
    replace<T>(args...);
  } else {
    add<T>(args...);
  }
}

template <typename T>
bool
Entity::has() {
  return components_.find(index<T>()) != components_.end();
}

template <typename E>
void 
Entity::trigger(E event) {
  if(has<Handler<E>>()) {
    get<Handler<E>>()->handler(event);
  }
}

template <typename T>
T*
Entity::get() const {
  auto item = components_.find(index<T>());
  if (item != components_.end()) {
    return static_cast<T*>(item->second);
  }

  throw std::runtime_error("Cannot get non-existent component");
}

template <typename T>
void 
Entity::rem() {
  rem(std::type_index(typeid(T)));
}

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
