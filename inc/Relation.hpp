//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_RELATION_HPP
#define INCLUDED_RELATION_HPP

//------------------------------------------------------------------------------

namespace corvid {

//------------------------------------------------------------------------------

template <typename... Attributes>
Relation<Attributes...>::Relation() 
: relation_(0) {
}

//------------------------------------------------------------------------------

template <typename... Attributes>
Relation<Attributes...>::Relation(const Relation& other) 
: relation_(other.relation_) {
}

//------------------------------------------------------------------------------

template <typename... Attributes>
Relation<Attributes...>::~Relation() {
}

//------------------------------------------------------------------------------

template <typename... Attributes>
template <typename Composite>
void 
Relation<Attributes...>::addComposite(Composite&& composite) {
  add(static_cast<Attributes>(composite)...);
}

//------------------------------------------------------------------------------

template <typename... Attributes>
void 
Relation<Attributes...>::add(const Attributes&... attributes) {
  add(std::make_tuple(attributes...));		
}

//------------------------------------------------------------------------------

template <typename... Attributes>
void 
Relation<Attributes...>::add(const std::tuple<Attributes...>& attributes) {
  relation_.push_back(attributes);		
  onAdd(attributes);
}

//------------------------------------------------------------------------------

template <typename... Attributes>
void 
Relation<Attributes...>::rem(const Attributes&... attributes) {
  rem(std::make_tuple(attributes...));		
}

//------------------------------------------------------------------------------

template <typename... Attributes>
void 
Relation<Attributes...>::rem(const std::tuple<Attributes...>& attributes) {
  auto attr = std::find(relation_.begin(), relation_.end(), attributes);

  if (attr != relation_.end()) {
    relation_.erase(attr);		
  }

  onRem(attributes);
}

//------------------------------------------------------------------------------

template <typename... Attributes>
template <typename...A>
Relation<A...>
Relation<Attributes...>::project() {
  Relation<A...> projection;
  for (auto it = relation_.begin(); it != relation_.end(); it++) {
    std::tuple<A...> p;
    projectTuple(*it, p);
    projection.add(p);
  }
  return projection;
}

//------------------------------------------------------------------------------

template <typename... Attributes>
template <typename...A>
Relation<A...>
Relation<Attributes...>::projectBases() {
  Relation<A...> projection;
  for (auto it = relation_.begin(); it != relation_.end(); it++) {
    std::tuple<A...> p;
    if (corvid::projectBases(*it, p)) {
      projection.add(p);
    }
  }
  return projection;
}

//------------------------------------------------------------------------------

template <typename... Attributes>
template <typename...A>
Relation<Attributes..., A...> 
Relation<Attributes...>::operator*(const Relation<A...>& relation) {
  Relation<Attributes..., A...> product;
  for (auto it = relation_.begin(); it != relation_.end(); it++) {
    for (auto other = relation.relation_.begin(); other != relation.relation_.end(); other++) {
      product.add(std::tuple_cat(*it, *other));
    }
  }

  return product;
}

//------------------------------------------------------------------------------

template <typename... Attributes>
Relation<Attributes...>& 
Relation<Attributes...>::operator+=(const Relation<Attributes...>& relation) {
  for (auto other = relation.relation_.begin(); other != relation.relation_.end(); other++) {
    add(*other);
  }

  return *this;
}

//------------------------------------------------------------------------------

template <typename... Attributes>
Relation<Attributes...> 
Relation<Attributes...>::operator+(const Relation<Attributes...>& relation) {
  Relation<Attributes...> sum = *this;
  sum += relation;
  return sum;
}

//------------------------------------------------------------------------------

template <typename... Attributes>
template <typename V>
V 
Relation<Attributes...>::visit(V visitor) {
  for (auto it = relation_.begin(); it != relation_.end(); it++) {
    apply(visitor, *it);
  }
}

//------------------------------------------------------------------------------

template <typename... Attributes>
template <typename P>
Relation<Attributes...> 
Relation<Attributes...>::select(P predicate) {
  Relation selection;
  for (auto it = relation_.begin(); it != relation_.end(); it++) {
    if (test(predicate, *it)) {
      selection.add(*it);
    }
  }
  return selection;
}

//------------------------------------------------------------------------------

template <typename... Attributes>
size_t
Relation<Attributes...>::count() const {
  return relation_.size();
}

//------------------------------------------------------------------------------

template <typename... Attributes>
void 
Relation<Attributes...>::onAdd(const std::tuple<Attributes...>& tuple) {
  // Do nothing
}

//------------------------------------------------------------------------------

template <typename... Attributes>
void 
Relation<Attributes...>::onRem(const std::tuple<Attributes...>& tuple) {
  // Do nothing
}

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#endif
