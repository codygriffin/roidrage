//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_TYPESET_H
#define INCLUDED_TYPESET_H

#ifndef INCLUDED_CSTRING
#include <cstring>
#define INCLUDED_CSTRING
#endif

//------------------------------------------------------------------------------

namespace corvid {

//------------------------------------------------------------------------------

// TypeSetImpl declaration.  Simply takes a list of unique types that make 
// up the set.
template <typename...T>
struct TypeSetImpl;

template <typename...T>
struct TypeSet;

// makeTypeSet(T...) returns a TypeSet containing its arguments
template <typename...T>
TypeSet<T...> makeTypeSet(T... t) {
  return TypeSet<T...>(t...);
}

// getType<N>(T) returns type N of TypeSet T, producing a compiler error 
// if it doesn't exist.
template <typename N, typename T> 
N& getType (T& typeSet);

// project(A, B) returns the projection of A onto B, given that TypeSet 
// B is a valid subset of TypSet A.
template <typename A, typename B>
void project(A& a, B& b);

// projectObject<A>(B) returns the projection of B onto A, given that TypeSet 
// A contains only base classes of B
template <typename A, typename B>
B projection(A& a);

// projectObject<A>(B) returns the projection of B onto A, given that TypeSet 
// A contains only base classes of B
template <typename A, typename B>
void projectObject(A& a, B& b);

//------------------------------------------------------------------------------

template <size_t N, typename T> 
struct TypeSetIndex;

template <size_t N, typename Head, typename...Tail> 
struct TypeSetIndex<N, TypeSet<Head, Tail...>> 
  : TypeSetIndex<N-1, TypeSet<Tail...>> {
};

template <typename Head, typename...Tail> 
struct TypeSetIndex<0, TypeSet<Head, Tail...>> {
  typedef Head Type;  
};

// getIndex<N>(T) returns the Nth item of TypeSet T
template <int N, typename T> 
typename TypeSetIndex<N, T>::Type& 
getIndex (T& typeSet);

//------------------------------------------------------------------------------
//
// TODO: Different levels of reference/const levels represent 
// concurrency safety/speed tradeoffs - best when inferred 
// correctly
// Apply the values of the TypeSet A to function V
template <typename V, typename A>
void apply(V& v, A& a);

// TODO: Different levels of reference/const levels represent 
// concurrency safety/speed tradeoffs - best when inferred 
// correctly
// 
template <typename...A>
bool test(bool (&predicate)(A...), TypeSet<A...>& a);

//------------------------------------------------------------------------------
/**
 * @class TypeSet<T...>  
 *
 */
} // namespace corvid

//------------------------------------------------------------------------------

#include <TypeSet.hpp>

#endif
