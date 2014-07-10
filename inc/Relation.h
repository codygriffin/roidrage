//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_RELATION_H
#define INCLUDED_RELATION_H

#ifndef INCLUDED_CORVID_H
#include <Corvid.h>
#endif

#ifndef INCLUDED_VECTOR
#include <vector>
#define INCLUDED_VECTOR
#endif

#ifndef INCLUDED_ALGORITHM
#include <algorithm>
#define INCLUDED_ALGORITHM
#endif

//------------------------------------------------------------------------------

namespace corvid { 

//------------------------------------------------------------------------------

template <typename... Attributes>
struct Relation {
  typedef std::tuple<Attributes...> RowType; 
	Relation();
	Relation(const Relation& other);
 ~Relation();

  // Allow us to create Relations from LiveRelations
	//Relation(const LiveRelation<Attributes...>& relation);
	//Relation<Attributes...>& operator=(const LiveRelation<Attributes...>& relation);

	void add(const Attributes&... attributes);
	void add(const RowType& attributes);

	void rem(const Attributes&... attributes);
	void rem(const RowType& attributes);
 
  template <typename Composite>
	void addComposite(Composite&& composite);

	template <typename...A>
  Relation<A...> project();

	template <typename...A>
  Relation<A...> projectBases();

	template <typename...A>
  Relation<Attributes..., A...> operator *(const Relation<A...>& relation);

  Relation&                     operator+=(const Relation&       relation);
  Relation                      operator +(const Relation&       relation);

  // Someday
  Relation                      operator-(const Relation&       relation);

	template <typename V>
  V visit(V visitor);

  template <typename P>
  Relation select(P predicate);

	size_t count() const;

  // TODO Re-implement product as a visit to make this private
  std::vector<RowType> relation_;

protected:
  // Hooks to modify any associated relations
  virtual void onAdd(const RowType& tuple);
  virtual void onRem(const RowType& tuple);

private:
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#include <Relation.hpp>

#endif
