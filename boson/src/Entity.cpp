#include "Entity.h"
#include <iostream>

using namespace boson;

//------------------------------------------------------------------------------

Entity::Entity(System& system, const std::string& name) 
: name_  (name)
, system_(system) {
  //add<EntityRef>(this);
}

//------------------------------------------------------------------------------

void 
Entity::clear() {
  system_.remove(name());
}

//------------------------------------------------------------------------------

void
Entity::rem(const std::type_index& i) {
  auto item = components_.find(i);
  if (item != components_.end()) {
    components_.erase(item);
    return;
  }

  throw std::runtime_error("Cannot remove non-existent component");
}

//------------------------------------------------------------------------------
