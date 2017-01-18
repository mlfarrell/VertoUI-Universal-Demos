//
//  WeatherData.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

class WeatherData
{
public:
  typedef std::shared_ptr<WeatherData> Pointer;
  typedef std::weak_ptr<WeatherData> WeakPointer;
  
  WeatherData(std::string locationName, std::string mainDescription, int degreesF);
  
  static WeatherData *current();
  
  inline std::string getLocationName() { return locationName; }
  inline std::string getMainDescription() { return mainDescription; }
  inline int getDegreesF() { return degreesF; }
  
protected:
  std::string locationName, mainDescription;
  int degreesF;
};

