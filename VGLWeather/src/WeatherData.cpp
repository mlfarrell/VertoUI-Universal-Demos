//
//  WeatherData.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#include "pch.h"
#include "WeatherData.h"

static WeatherData *currentWeatherData = nullptr;

WeatherData::WeatherData(std::string locationName, std::string mainDescription, int degreesF)
  : locationName(locationName), mainDescription(mainDescription), degreesF(degreesF)
{
  currentWeatherData = this;
}

WeatherData *WeatherData::current()
{
  return currentWeatherData;
}
