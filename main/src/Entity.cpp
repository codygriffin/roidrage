#include <Entity.h>

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

Entity& 
System::entity(const std::string& name) {
  auto e = entities_.find(name);
  if (e == entities_.end()) {
    auto e = entities_.emplace(name, Entity(*this, name));
    return e.first->second;
  }

  return e->second;
}

void 
System::remove(const std::string& name) {
  auto e = entities_.find(name);
  if (e != entities_.end()) {
    for (auto kv : indices_) {
      kv.second->remove(e->second);
    }
    entities_.erase(e);    
    return;
  }

  throw std::runtime_error("Cannot remove non-existent entity");
}

Entity& 
System::entity() {
  auto randchar = []() -> char
  {
      const char charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
      const size_t max_index = (sizeof(charset) - 1);
      return charset[ rand() % max_index ];
  };
  std::string name(10,0);
  std::generate_n(name.begin(), name.length(), randchar);
  auto e = entities_.find(name);
  if (e == entities_.end()) {
    auto e = entities_.emplace(name, Entity(*this, name));
    return e.first->second;
  }

  return e->second;
}

void
System::indexEntity(Entity& e) {
  for (auto kv : indices_) {
    kv.second->index(e);
  }
}
