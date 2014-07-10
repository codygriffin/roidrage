//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_POPULATION_H
#define INCLUDED_POPULATION_H

#ifndef INCLUDED_FUNCTIONAL
#include <functional>
#define INCLUDED_FUNCTIONAL
#endif

#ifndef INCLUDED_RELATION
#include "Relation.h"
#endif

//------------------------------------------------------------------------------

namespace pronghorn { 

//------------------------------------------------------------------------------

template <typename...Types>
struct Population {

  void clear();

  template<typename E>
  void clear();

  template <typename E>
  void add(E entity);

  template <typename E>
  void remove(E entity);

  template <typename...Traits>
  corvid::Relation<Traits...> project();

  template <typename...Traits, typename...Traits2>
  corvid::Relation<Traits...> project2();

  template <typename...Traits>
  corvid::Relation<Traits..., Traits...> product();

  template <typename...V>
  void visit(void (*pVisitor)(V...));

  template <typename...V>
  void pair(void (*pVisitor)(V...));

  template <typename E>
  corvid::Relation<E>& getRelation();

private:
  std::tuple<
    corvid::Relation<Types>...
  > population_;
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#include "Population.hpp"

#endif
