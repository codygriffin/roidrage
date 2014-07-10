//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_TYPESET_HPP
#define INCLUDED_TYPESET_HPP

#ifndef INCLUDED_TYPETRAITS
#include <type_traits>
#define INCLUDED_TYPETRAITS
#endif

//------------------------------------------------------------------------------

namespace corvid {

//------------------------------------------------------------------------------

// We recursively define the TypeSet as a list of types.
// TODO: type uniqueness constraint
template <typename This, typename...Comp>
struct TypeSet<This, Comp...> {
  TypeSet()  {}

  TypeSet(This head, Comp...tail) 
  : value(head)
  , remainder(tail...) {
  }

  This             value;
  TypeSet<Comp...> remainder;
  static const size_t card = 1 + TypeSet<Comp...>::card;
};

// Rather than terminating the base-case at the empty TypeSet, we 
// will instead terminate at a TypeSet with cardinality 1.  This 
// prevents the compiler from giving a size to an empty type, so
// the TypeSet only takes up as much memory as needed.
template <typename Last>
struct TypeSet<Last> {
  TypeSet()  {}

  TypeSet(Last last) 
  : value(last) {}

  operator Last () {
    return value;
  }

  Last value;
  static const size_t card = 1;
};

// The empty case.  We provide this for completeness.  It is
// a valid project for all TypeSets, and has cardinality zero,
// but any getType function will produce a compile-time error.
template <>
struct TypeSet<> {
};

template <typename T> 
struct TypeSetElements;

template <typename Head, typename...Tail> 
struct TypeSetElements<TypeSet<Head, Tail...>> {
  template <typename N>
  typename std::enable_if< std::is_same<N, Head>::value, N&>::type
  static getType(TypeSet<Head, Tail...>& typeSet) {
    return typeSet.value;
  }

  template <typename N>
  typename std::enable_if<!std::is_same<N, Head>::value, N&>::type
  static getType(TypeSet<Head, Tail...>& typeSet) {
    return TypeSetElements<TypeSet<Tail...>>::template getType<N>(typeSet.remainder);
  }

  template <typename N>
  TypeSet<Head, Tail...>
  static project(N& from) {
    typedef TypeSetElements<N>                ElementsOfN;
    typedef TypeSetElements<TypeSet<Tail...>> Remainder;
    return TypeSet<Head, Tail...>(
      ElementsOfN::template getType<Head>(from), 
      Remainder::template project<N>(from));
  }

  template <typename E>
  TypeSet<Head, Tail...>
  static projectObject(E& entity) {
    typedef TypeSetElements<TypeSet<Tail...>> Remainder;
    return TypeSet<Head, Tail...>(
      static_cast<Head>(entity), 
      Remainder::template projectObject<E>(entity));
  }
};

template <typename Last> 
struct TypeSetElements<TypeSet<Last>> {
  template <typename N>
  typename std::enable_if< std::is_same<N, Last>::value, N&>::type
  static getType(TypeSet<Last>& typeSet) {
    return typeSet.value;
  }

  template <typename N>
  typename std::enable_if<!std::is_same<N, Last>::value, N&>::type
  static getType(TypeSet<Last>& typeSet) {
    static_assert(std::is_same<N, Last>::value, "No element N in TypeSet");
  }

  template <typename N>
  TypeSet<Last>
  static project(N& from) {
    typedef TypeSetElements<N> ElementsOfN;
    return TypeSet<Last>(ElementsOfN::template getType<Last>(from));
  }

  template <typename E>
  TypeSet<Last>
  static projectObject(E& entity) {
    return TypeSet<Last>(static_cast<Last>(entity)); 
  }
};

//------------------------------------------------------------------------------

template <typename N, typename T> 
N& getType (T& typeSet) {
  return TypeSetElements<T>::template getType<N>(typeSet);
}

template <int N, typename T> 
typename TypeSetIndex<N, T>::Type& 
getIndex (T& typeSet) {
  return getType<typename TypeSetIndex<N, T>::Type>(typeSet);
}

template <typename T> 
size_t card (T& typeSet) {
  return TypeSetElements<T>::card(typeSet);
}

template <typename T> 
size_t size (T& typeSet) {
  return TypeSetElements<T>::size(typeSet);
}

template <typename A, typename B>
void project(A& a, B& b) {
  b = TypeSetElements<B>::project(a);
}

template <typename A, typename B>
B projection(A& a) {
  return TypeSetElements<B>::project(a);
}

template <typename A, typename B> 
void projectObject(A& a, B& b) {
  b = TypeSetElements<B>::template projectObject<A>(a);
}

template <typename A, typename B> 
void project(TypeSet<A>& a, B& b) {
  projectObject<A, B>(a, b);
}

//------------------------------------------------------------------------------

template <int N> struct ApplyTypeSet;
template <int N> struct TestTypeSet;

//------------------------------------------------------------------------------

template <typename V, typename A>
void apply(V& v, A& a) {
  ApplyTypeSet<A::card>::apply(&v, a);
}

//------------------------------------------------------------------------------

template <typename...A>
bool test(bool (&f)(A...), TypeSet<A...>& a) {
  return TestTypeSet<TypeSet<A...>::card>::test(f, a);
}

//------------------------------------------------------------------------------

template <int N> 
struct ApplyTypeSet { 
    template <class V, class... ArgsV, class... Args> 
    static void apply(V*               pVisitor,
                      TypeSet<ArgsV...>& data,
                      Args...            args) {
    ApplyTypeSet<N-1>::apply(pVisitor, data, getIndex<N-1>(data), args...);
  }
};

//------------------------------------------------------------------------------

template <> 
struct ApplyTypeSet<0> {
  template <class V, class... ArgsV, class... Args> 
  static void apply(V*                 pVisitor,
                    TypeSet<ArgsV...>& data,
                    Args...            args) {
    (*pVisitor)(args...);
  }
};

//------------------------------------------------------------------------------

template <int N> 
struct TestTypeSet { 
  template <class... ArgsP, class... Args> 
  static bool test(bool (&predicate)(ArgsP...),
                   TypeSet<ArgsP...>&  data,
                   Args...             args) {
    return TestTypeSet<N-1>::test(predicate, data, getIndex<N-1>(data), args...);
  }
};

//------------------------------------------------------------------------------

template <> 
struct TestTypeSet<0> {
  template <class... ArgsP, class... Args> 
  static bool test(bool (&predicate)(ArgsP...), 
                   TypeSet<ArgsP...>& data,
                   Args...            args) {
    return predicate(args...);
  }
};

//------------------------------------------------------------------------------

template <typename T, typename N>
struct Rename {
  Rename()
  :  value_() {
  }

  Rename(const T& value)
  :  value_(value) {
  }

  T& operator=(const T& value) {
    return value_;
  }

  operator T () {
    return value_;
  }

  typedef T Type;  
  T         value_;
};

//------------------------------------------------------------------------------

template <int N>
struct Tag;

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#endif
