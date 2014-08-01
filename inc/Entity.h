//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_ENTITY_H
#define INCLUDED_ENTITY_H

#include "Corvid.h"
#include <string>

#include <typeinfo>
#include <typeindex>

#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <set>

//------------------------------------------------------------------------------

struct System;
struct IComponent {
  virtual ~IComponent() {}
  virtual std::string toString() = 0;
};

template <typename T>
struct Component : public IComponent {
  virtual ~Component() {}
  // knows what type it is
  virtual std::string toString() {
    return "???";
  }

  bool operator<(const Component& other) const {
    return this < &other; 
  }
};
#define COMPONENT(n) struct n : public Component<n>

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

struct Entity {
  Entity(System& system, const std::string& name) : name_(name), system_(system) {
  }
 
  void clear() {
    system_.remove(name());
  }
  
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
    corvid::apply(pVisitor, c);
  } 
}

template <typename...Args>
template <typename T>
void 
Index<Args...>::foreach(T& context, void (T::*pVisitor)(Args*...)) {
  for (auto c : index_) {
    corvid::apply(context, pVisitor, c);
  } 
}

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

//------------------------------------------------------------------------------

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
    //delete item->second;
    new (item->second) T(args...);
    //system_.indexEntity(*this);
    return;
  }

  throw std::runtime_error("Cannot replace non-existent component");
}

template <typename T>
bool
Entity::has() {
  return components_.find(index<T>()) != components_.end();
}

template <typename T>
T*
Entity::get() {
  auto item = components_.find(index<T>());
  if (item != components_.end()) {
    return static_cast<T*>(item->second);
  }

  throw std::runtime_error("Cannot get non-existent component");
}

#endif
