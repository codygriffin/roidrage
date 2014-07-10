//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#ifndef INCLUDED_VALIDATE_H
#define INCLUDED_VALIDATE_H

//------------------------------------------------------------------------------

namespace roidrage { 

//------------------------------------------------------------------------------

struct Validate  {
  Validate(bool isValid = true);
  virtual ~Validate();
  
  void validate();
  void invalidate();
  bool isValid() const;

protected:
  bool onValidate();
  void onInvalidate();

private:
  bool isValid_;
};

//------------------------------------------------------------------------------

#define VALIDATE(x) if (!x->isValid()) { \
    return; \
  } 

} // namespace roidrage

//------------------------------------------------------------------------------

#endif
