//------------------------------------------------------------------------------
//
// Copyright (C) 2014 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_COMPONENT_H
#define INCLUDED_COMPONENT_H

#include <string>

//------------------------------------------------------------------------------

namespace boson {

//------------------------------------------------------------------------------

struct Entity;
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

template<typename E>
struct Handler : public Component<Handler<E>> {
  Handler(std::function<void (E)> fun) : handler(fun) {}
  std::function<void (E)> handler;
}; 

struct EntityRef : public Component<EntityRef> {
  Entity* entity;
  EntityRef(Entity* e) : entity(e) {}
};

//------------------------------------------------------------------------------

}

//------------------------------------------------------------------------------

#endif
