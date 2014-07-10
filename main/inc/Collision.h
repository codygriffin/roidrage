//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_COLLISION_H
#define INCLUDED_COLLISION_H

//------------------------------------------------------------------------------

namespace pronghorn { 

//------------------------------------------------------------------------------

template <typename E>
struct This {};

template <typename A, typename B, typename...X>
struct CollisionPair {
  virtual void onCollideWith(CollisionPair*, X...) = 0;
  virtual void onCollideWith(A*, X...) = 0;
  virtual void onCollideWith(B*, X...) = 0;
};

template <typename A, typename B, typename...X>
struct CollisionPair<This<A>, B, X...> : public CollisionPair<A, B, X...> 
                                       , public CollisionPair<B, A, X...> {
  void onCollideWith(CollisionPair<A, B, X...>* pBase, X...xs) {
    pBase->onCollideWith(static_cast<A*>(this), xs...);
  }
  void onCollideWith(CollisionPair<B, A, X...>* pBase, X...xs) {
    pBase->onCollideWith(static_cast<A*>(this), xs...);
  }

  #ifdef COLLISION_DEFAULT
  virtual void onCollideWith(A*, X...) {};
  virtual void onCollideWith(B*, X...) {};
  #endif
};

template <typename A, typename B, typename...X>
struct CollisionPair<A, This<B>, X...> : public CollisionPair<A, B, X...> 
                                       , public CollisionPair<B, A, X...> {
  void onCollideWith(CollisionPair<A, B>* pBase, X...xs) {
    pBase->onCollideWith(static_cast<B*>(this), xs...);
  }
  void onCollideWith(CollisionPair<B, A>* pBase, X...xs) {
    pBase->onCollideWith(static_cast<B*>(this), xs...);
  }

  #ifdef COLLISION_DEFAULT
  virtual void onCollideWith(A*, X...) {};
  virtual void onCollideWith(B*, X...) {};
  #endif
};

//------------------------------------------------------------------------------

} // namespace corvid

//------------------------------------------------------------------------------

#endif
