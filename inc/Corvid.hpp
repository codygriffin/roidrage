//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_CORVID_HPP
#define INCLUDED_CORVID_HPP

//------------------------------------------------------------------------------

namespace corvid {

//------------------------------------------------------------------------------

template <typename E, typename T> 
inline E& 
getType (T& tuple) {
  return TupleGetType<
    std::tuple_size<T>::value-1, 
    T
  >::template getType<E>(tuple);
}

//------------------------------------------------------------------------------
// Get a type from tuple Tuple that is is_convertible to type Base
template <typename Base, typename Tuple>
inline typename MatchBaseType<std::tuple_size<Tuple>::value-1, Tuple, Base>::Type& 
getBase (Tuple& tuple) {
  return TupleGetBase<
    std::tuple_size<Tuple>::value-1, 
    Tuple
  >::template getType<Base>(tuple);
}


//------------------------------------------------------------------------------

template <typename A, typename B>
inline void 
projectTuple(A& a, B& b) {
  TupleProject<
    std::tuple_size<B>::value-1, 
    B
  >::project(a, b);
}

//------------------------------------------------------------------------------

template <typename TupleA, typename TupleB>
inline bool 
projectBases(TupleA& a, TupleB& b) {
  return TupleBaseProject<
    std::tuple_size<TupleB>::value-1, 
    TupleB
  >::project(a, b);
}

//------------------------------------------------------------------------------

template <typename V, typename A>
inline void 
apply(V& v, A& a) {
  TupleApply<
    std::tuple_size<A>::value
  >::apply(&v, a);
}

//------------------------------------------------------------------------------

template <typename A, typename R, typename...S>
inline R 
apply(R (*f)(S...), A& a) {
  return TupleApply<
    std::tuple_size<A>::value
  >::apply(f, a);
}

//------------------------------------------------------------------------------

template <typename T, typename A, typename R, typename...S>
inline R 
apply(T& t, R (T::*f)(S...), A& a) {
  return TupleApply<
    std::tuple_size<A>::value
  >::apply(t, f, a);
}

//------------------------------------------------------------------------------

template <typename P, typename A>
inline bool 
test(P& p, A& a) {
  return TupleTest<
    std::tuple_size<A>::value
  >::test(&p, a);
}

//------------------------------------------------------------------------------

template <int N> 
struct TupleApply { 
    template <typename V, typename... ArgsV, typename... Args> 
    inline static void 
    apply(V*                    pVisitor,
          std::tuple<ArgsV...>& data,
          Args...               args) {
    TupleApply<N-1>::apply(pVisitor, 
                           data, 
                           std::get<N-1>(data), 
                           args...);
  }

  template <typename R, typename... ArgsV, typename... Args> 
  inline static R 
  apply(R (*pVisitor)(ArgsV...),
        std::tuple<ArgsV...>& data,
        Args...               args) {
    return TupleApply<N-1>::apply(pVisitor, 
                                  data, 
                                  std::get<N-1>(data), 
                                  args...);
  }

  template <typename T, typename R, typename... ArgsV, typename... Args> 
  inline static R 
  apply(T& t, R (T::*pVisitor)(ArgsV...),
        std::tuple<ArgsV...>& data,
        Args...               args) {
    return TupleApply<N-1>::apply(t,
                                  pVisitor, 
                                  data, 
                                  std::get<N-1>(data), 
                                  args...);
  }
};

//------------------------------------------------------------------------------

template <> 
struct TupleApply<0> {
  template <typename V, typename... ArgsV, typename... Args> 
  inline static void 
  apply(V*                    pVisitor,
        std::tuple<ArgsV...>& data,
        Args...               args) {
    (*pVisitor)(args...);
  }

  template <typename R, typename... ArgsV, typename... Args> 
  inline static R 
  apply(R (*pVisitor)(ArgsV...),
        std::tuple<ArgsV...>& data,
        Args...               args) {
    return (*pVisitor)(args...);
  }

  template <typename T, typename R, typename... ArgsV, typename... Args> 
  inline static R 
  apply(T& t, R (T::*pVisitor)(ArgsV...),
        std::tuple<ArgsV...>& data,
        Args...               args) {
    return (t.*pVisitor)(args...);
  }
};

//------------------------------------------------------------------------------

template <int N> 
struct TupleTest { 
  template <typename P, typename... ArgsP, typename... Args> 
  inline static bool 
  test(P*                     pPredicate,
       std::tuple<ArgsP...>&  data,
       Args...                args) {
    return TupleTest<N-1>::test(pPredicate, 
                                data, 
                                std::get<N-1>(data), 
                                args...);
  }
};

//------------------------------------------------------------------------------

template <> 
struct TupleTest<0> {
  template <typename P, typename... ArgsP, typename... Args> 
  inline static bool 
  test(P*                    pPredicate, 
       std::tuple<ArgsP...>& data,
       Args...               args) {
    return (*pPredicate)(args...);
  }
};

//------------------------------------------------------------------------------
// Recursively iterate over tuple
template <int N, typename A> 
struct TupleGetType {  
  typedef typename std::tuple_element<N, A>::type ElementN;

  template <typename T>
  inline static typename std::enable_if<std::is_same<T, ElementN>::value, T&>::type
  getType(A& tuple) {
    return std::get<N>(tuple);
  }

  template <typename T>
  inline static typename std::enable_if<!std::is_same<T, ElementN>::value, T&>::type
  getType(A& tuple) {
    return TupleGetType<N-1, A>::template getType<T>(tuple);
  }
};

//------------------------------------------------------------------------------
// Terminate at first element
template <typename A> 
struct TupleGetType<0, A> {
  typedef typename std::tuple_element<0, A>::type ElementN;

  template <typename T>
  inline static typename std::enable_if< std::is_same<T, ElementN>::value, T&>::type
  getType(A& tuple) {
    return std::get<0>(tuple);
  }

  template <typename T>
  inline static typename std::enable_if<!std::is_same<T, ElementN>::value, T&>::type
  getType(A& tuple) {
    static_assert(std::is_same<T, ElementN>::value, 
                  "No element of type T in std::tuple");
  }
};

//------------------------------------------------------------------------------
// Recursively iterate over tuple
template <int N, typename A, typename T> 
struct TupleHasType {  
  typedef typename std::tuple_element<N, A>::type ElementN;
};

//------------------------------------------------------------------------------
// Terminate at first element
template <typename A, typename T> 
struct TupleHasType<0, A, T> {
  typedef typename std::tuple_element<0, A>::type ElementN;
};

//------------------------------------------------------------------------------
// Recursively iterate over tuple
template <int N, typename Tuple> 
struct TupleGetBase {  
  typedef typename std::tuple_element<N, Tuple>::type ElementN;

  template <typename Base>
  inline static typename std::enable_if<
    std::is_convertible<ElementN, Base>::value, 
    ElementN&
  >::type
  getType(Tuple& tuple) {
    return std::get<N>(tuple);
  }

  template <typename Base>
  inline static typename std::enable_if<
    !std::is_convertible<ElementN, Base>::value, 
    typename MatchBaseType<N-1, Tuple, Base>::Type&
  >::type
  getType(Tuple& tuple) {
    return TupleGetBase<N-1, Tuple>::template getType<Base>(tuple);
  }
};

//------------------------------------------------------------------------------
// Terminate at first element
template <typename Tuple> 
struct TupleGetBase<0, Tuple> {
  typedef typename std::tuple_element<0, Tuple>::type ElementN;

  template <typename Base>
  inline static typename std::enable_if< 
    std::is_convertible< ElementN, Base>::value, 
    ElementN&
  >::type
  getType(Tuple& tuple) {
    return std::get<0>(tuple);
  }

  template <typename Base>
  inline static typename std::enable_if<
    !std::is_convertible<ElementN, Base>::value, 
    ElementN&
  >::type
  getType(Tuple& tuple) {
    static_assert(!std::is_convertible<ElementN, Base>::value, 
                  "No element with Base in Tuple");
  }
};

//------------------------------------------------------------------------------
// Recursively iterate over tuple
template <int N, typename A, typename T> 
struct TupleHasBase {  
  typedef typename std::tuple_element<N, A>::type ElementN;
  static const bool hasBase = TupleHasBase<N-1, A, T>::hasBase || std::is_convertible<ElementN, T>::value;
};

//------------------------------------------------------------------------------
// Terminate at first element
template <typename A, typename T> 
struct TupleHasBase<0, A, T> {
  typedef typename std::tuple_element<0, A>::type ElementN;
  static const bool hasBase = std::is_convertible<ElementN, T>::value;
};

//------------------------------------------------------------------------------

template <int N, typename Tuple, typename Base> 
struct MatchBaseType {
  typedef typename std::tuple_element<N, Tuple>::type     ElementN;
  typedef typename MatchBaseType<N-1, Tuple, Base>::Type ElementM;

  typedef typename std::conditional<
    std::is_convertible<ElementN, Base>::value, 
    ElementN, 
    ElementM
  >::type Type;
};

//------------------------------------------------------------------------------

template <typename Tuple, typename Base>
struct MatchBaseType<0, Tuple, Base> {
  typedef typename std::tuple_element<0, Tuple>::type Type;
  // Substitution error?
};

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Recursively iterate over tuple
template <int N, typename B> 
struct TupleProject {  
  typedef typename std::tuple_element<N, B>::type ElementN;

  template <typename A>
  inline static void 
  project(A& a, B& b) {
    getType<ElementN>(b) = getType<ElementN>(a);
    TupleProject<N-1, B>::project(a, b);
  }
};

//------------------------------------------------------------------------------
// Terminate at first element
template <typename B> 
struct TupleProject<0, B> {
  typedef typename std::tuple_element<0, B>::type ElementN;

  template <typename A>
  inline static void 
  project(A& a, B& b) {
    getType<ElementN>(b) = getType<ElementN>(a);
  }
};

//------------------------------------------------------------------------------
// Recursively iterate over tuple
template <int N, typename B> 
struct TupleBaseProject {  
  typedef typename std::tuple_element<N, B>::type ElementN;

  template <typename A>
  inline static typename std::enable_if<
    TupleHasBase<std::tuple_size<A>::value-1, A, ElementN>::hasBase, 
    bool
  >::type
  project(A& a, B& b) {
    getBase<ElementN>(b) = getBase<ElementN>(a);
    return TupleBaseProject<N-1, B>::project(a, b);
  }

  template <typename A>
  inline static typename std::enable_if<
    !TupleHasBase<std::tuple_size<A>::value-1, A, ElementN>::hasBase, 
    bool
  >::type
  project(A& a, B& b) {
    return false;
  }
};

//------------------------------------------------------------------------------
// Terminate at first element
template <typename B> 
struct TupleBaseProject<0, B> {
  typedef typename std::tuple_element<0, B>::type ElementN;

  template <typename A>
  inline static typename std::enable_if<
    TupleHasBase<std::tuple_size<A>::value-1, A, ElementN>::hasBase, 
    bool
  >::type
  project(A& a, B& b) {
    getBase<ElementN>(b) = getBase<ElementN>(a);
    return true;
  }

  template <typename A>
  inline static typename std::enable_if<
    !TupleHasBase<std::tuple_size<A>::value-1, A, ElementN>::hasBase, 
    bool
  >::type
  project(A& a, B& b) {
    return false;
  }
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#endif
