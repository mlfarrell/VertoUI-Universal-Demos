//
//  WeatherSetupViewController.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#pragma once

#include "ViewController.h"
#include "TextEntry.h"
#include "WeatherData.h"

class WeatherSetupViewController : public vui::ViewController
{
public:
  typedef std::shared_ptr<WeatherSetupViewController> Pointer;
  typedef std::weak_ptr<WeatherSetupViewController> WeakPointer;

  virtual void loadView() override;
  
  void onGoPressed(vui::Control::Pointer sender);
  
  inline WeatherData::Pointer getWeatherData() { return weatherData; }
    
protected:
  void showWeather();
  
  vui::TextEntry::Pointer zipCodeEntry;
  WeatherData::Pointer weatherData;
  bool busy = false;
};
