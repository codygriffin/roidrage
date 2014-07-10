//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

namespace thunk {

//------------------------------------------------------------------------------

inline 
void packListItem(List* pList, List*& l, Scope* pScope) {
  if (pList && pList->get(0)) {
    l = as<List>(pList->get(0)->eval(pScope));
  } else {
    l = nil;
  }
}

inline 
void packListItem(List* pList, std::string& s, Scope* pScope) {
  if (pList && pList->get(0)) {
    s = as<Str>(pList->get(0)->eval(pScope))->str;
  } else {
    s = "";
  }
}

inline 
void packListItem(List* pList, int& i, Scope* pScope) {
  if (pList && pList->get(0)) {
    i = as<Int>(pList->get(0)->eval(pScope))->num;
  } else {
    i = 0;
  }
}

inline 
void packListItem(List* pList, float& i, Scope* pScope) {
  if (pList && pList->get(0)) {
    i = as<Float>(pList->get(0)->eval(pScope))->num;
  } else {
    i = 0;
  }
}

//------------------------------------------------------------------------------

template <int N, typename A, int M> 
struct FillTupleWithList { 
  typedef typename std::tuple_element<N, A>::type ElementN;

  inline static void 
  fill (A& tuple, List* pList, Scope* pScope) {
    packListItem(pList, std::get<N>(tuple), pScope);
    FillTupleWithList<N+1, A, M>::fill(tuple, pList->pTail, pScope);
  }
};

template <typename A, int M> 
struct FillTupleWithList<M, A, M> {
  typedef typename std::tuple_element<M, A>::type ElementM;

  inline static void 
  fill (A& tuple, List* pList, Scope* pScope) {
    packListItem(pList, std::get<M>(tuple), pScope);
  }
};

template <typename A>
inline void 
fillTupleWithList(A& a, List* pList, Scope* pScope) {
  FillTupleWithList<
    0, A, std::tuple_size<A>::value - 1
  >::fill(a, pList, pScope);
}

//------------------------------------------------------------------------------

template <typename...S>
Fun* expose(void (*f)(S...)) {
  return new Fun([=](List* pArgs, Scope* pScope) {
    std::tuple<S...> t;
    fillTupleWithList(t, pArgs, pScope);
    corvid::apply(f, t);
    return nil;
  });
}

template <typename R, typename...S>
Fun* expose(R (*f)(S...)) {
  return new Fun([=](List* pArgs, Scope* pScope) {
    std::tuple<S...> t;
    fillTupleWithList(t, pArgs, pScope);
    return new typename FromNative<R>::Type(corvid::apply(f, t));
  });
}

}
