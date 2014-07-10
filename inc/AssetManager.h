//------------------------------------------------------------------------------
//
// Copyright (C) 2012 Cody Griffin (cody.m.griffin@gmail.com)
// 

#ifndef INCLUDED_ASSETMANAGER_H
#define INCLUDED_ASSETMANAGER_H

#ifndef INCLUDED_MEMORY
#include <memory>
#define INCLUDED_MEMORY
#endif

//------------------------------------------------------------------------------

namespace pronghorn { 

//------------------------------------------------------------------------------

struct AssetManager  {
  static std::unique_ptr<char[]> loadBitmap(const std::string& str, unsigned& width, unsigned& height);
  static std::unique_ptr<char[]> loadText(const std::string& str);
};

//------------------------------------------------------------------------------

} // namespaces

//------------------------------------------------------------------------------

#endif
