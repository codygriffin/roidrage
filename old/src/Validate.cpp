//------------------------------------------------------------------------------
//
// Copyright (C) 2013 Cody Griffin (cody.m.griffin@gmail.com)
// 
//------------------------------------------------------------------------------

#include "Validate.h"

//------------------------------------------------------------------------------

using namespace roidrage;

//------------------------------------------------------------------------------

Validate::Validate(bool isValid) 
  : isValid_(isValid) {
}

Validate::~Validate() {
}

void
Validate::validate() {
  isValid_ = onValidate();
}

void
Validate::invalidate() {
  onInvalidate();
  isValid_ = false;
}

bool Validate::isValid() const {
  return isValid_;
}

bool Validate::onValidate() {
  return true;
}

void Validate::onInvalidate() {
}

//------------------------------------------------------------------------------
