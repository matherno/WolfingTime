#pragma once

#include "WolfMap.h"

/*
*   
*/

class MapImporter
  {
public:
  static WolfMapPtr importMap(const std::string& filePath);
  };
