//
//  WeatherRenderer.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/18/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#pragma once

#include <memory>
#include <functional>
#include "Scene.h"
#include "WeatherData.h"

///Provides the common code between holographic & non-holographic versions of the app
/// to do the 3D rendering of the weather "display"

class WeatherRenderer
{
public:
  typedef std::shared_ptr<WeatherRenderer> Pointer;
  typedef std::weak_ptr<WeatherRenderer> WeakPointer;

  WeatherRenderer(WeatherData::Pointer weatherData, std::function<void()> onLoadingFinished);
  
  virtual void render();
  
protected:
  std::string assetNameForWeatherCondition(std::string condition);
  
  WeatherData::Pointer weatherData;
  vom::Scene::Pointer scene;
  vom::EntityModel::Pointer locationText, temperatureText;
  float time0 = 0;
};
