//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_POPULATION_HPP
#define INCLUDED_POPULATION_HPP

#include "Validate.h"

//------------------------------------------------------------------------------

namespace pronghorn {

//------------------------------------------------------------------------------

template <int N, typename A>
struct PopulationClear {
  typedef typename std::tuple_element<N, A>::type ElementN;

  template <typename...Traits>
  static void clear(A& population) {
    corvid::getType<ElementN>(population).relation_.clear();
    PopulationClear<N-1, A>::template clear(population); 
  }
};

//------------------------------------------------------------------------------

template <typename A>
struct PopulationClear<0, A> {
  typedef typename std::tuple_element<0, A>::type ElementN;

  template <typename...Traits>
  static void clear(A& population) {
    corvid::getType<ElementN>(population).relation_.clear();
  }
};

//------------------------------------------------------------------------------

template <int N, typename A>
struct PopulationProject {
  typedef typename std::tuple_element<N, A>::type ElementN;

  template <typename...Traits>
  static void project(corvid::Relation<Traits...>& projection, A& population) {
    projection += corvid::getType<ElementN>(population).template projectBases<Traits...>();
    PopulationProject<N-1, A>::template project(projection, population); 
  }
};

//------------------------------------------------------------------------------

template <typename A>
struct PopulationProject<0, A> {
  typedef typename std::tuple_element<0, A>::type ElementN;

  template <typename...Traits>
  static void project(corvid::Relation<Traits...>& projection, A& population) {
    projection += corvid::getType<ElementN>(population).template projectBases<Traits...>();
  }
};

//------------------------------------------------------------------------------

template <typename...Types>
void
Population<Types...>::clear() {
  PopulationClear<
    sizeof...(Types)-1,
    std::tuple<
      corvid::Relation<Types>...
    >
  >::clear(population_);
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename E>
void
Population<Types...>::clear() {
  getRelation<E>().relation_.clear();
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename E>
void 
Population<Types...>::add(E entity) {
  getRelation<E>().add(entity);
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename E>
void 
Population<Types...>::remove(E entity) {
  getRelation<E>().rem(entity);
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename...Traits>
corvid::Relation<Traits...> 
Population<Types...>::project() {
  // TODO Memoization
  corvid::Relation<Traits...> projection; 

  PopulationProject<
    sizeof...(Types)-1,
    std::tuple<
      corvid::Relation<Types>...
    >
  >::project(projection, population_);

  return projection;
}

//------------------------------------------------------------------------------
// This is a helper for "pair"
template <int N, typename...T>
struct Project2;

template <int N, typename H, typename...T>
struct Project2<N, H, T...> {
  typedef typename Project2<N-1, T...>::RelationType RelationType;

  template <typename P> 
  static RelationType 
  project2(P* pPopulation) {
    return Project2<N-1, T...>::template project2<P>(pPopulation);
  }
};

// Terminating at zero makes more sense, but I get an ambiguity :(
template <typename H, typename...T>
struct Project2<1, H, T...> {
  typedef corvid::Relation<T...> RelationType; 
  template <typename P> 
  static RelationType 
  project2(P* pPopulation) {
    return pPopulation->template project<T...>();
  }
};

//------------------------------------------------------------------------------

template <typename...Types>
template <typename...Traits>
corvid::Relation<Traits..., Traits...> 
Population<Types...>::product() {
  auto projection = project<Traits...>();
  return projection * projection;
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename...V>
void 
Population<Types...>::visit(void (*pVisitor)(V...)) {
  project<V...>().visit(pVisitor);
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename...V>
void 
Population<Types...>::pair(void (*pVisitor)(V...)) {
  // project2 means we only project the first half of the 
  // V... pack (its repeated)
  typedef Project2<sizeof...(V)/2, V...> Projection;
  typename Projection::RelationType projection = Projection::project2(this);
  // then we do our cartesian product and visit
  (projection * projection).visit(pVisitor);  
}

//------------------------------------------------------------------------------

template <typename...Types>
template <typename E>
corvid::Relation<E>& 
Population<Types...>::getRelation() {
  return corvid::getType<corvid::Relation<E>>(population_);
}

//------------------------------------------------------------------------------

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
