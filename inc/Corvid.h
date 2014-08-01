//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_CORVID_H
#define INCLUDED_CORVID_H

#ifndef INCLUDED_TUPLE
#include <tuple>
#define INCLUDED_TUPLE
#endif

//------------------------------------------------------------------------------

namespace corvid { 

//------------------------------------------------------------------------------

template <int N>                         
struct TupleApply;

template <int N>                         
struct TupleApplyBases;

template <int N>                         
struct TupleTest;

template <int N>                         
struct TupleWeakTest;

template <int N, typename Tuple, typename Base> 
struct MatchBaseType;

template <int N, typename A>             
struct TupleGetType;

template <int N, typename A>             
struct TupleGetBase;

template <int N, typename B>             
struct TupleProject;

template <int N, typename B>             
struct TupleBaseProject;

template <typename V, typename A>
void apply(V& v, A& a);

template <typename A, typename R, typename...S>
R    apply(R (*f)(S...), A& a);

template <typename T, typename A, typename R, typename...S>
R    apply(T& t, R (T::*f)(S...), A& a);

template <typename P, typename A>
bool test(P& p, A& a);

template <typename T, typename A> 
T& getType (A& tuple);

template <typename Base, typename Tuple> 
typename MatchBaseType<std::tuple_size<Tuple>::value-1, Tuple, Base>::Type& 
getBase (Tuple& tuple);

template <typename A, typename B>
void projectTuple(A& a, B& b);

template <typename A, typename B>
bool projectBases(A& a, B& b);

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#include <Corvid.hpp>

#endif
