//
//  WeatherViewController.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#pragma once

#include "ViewController.h"
#include "WeatherSetupViewController.h"
#include "Button.h"
#include "WeatherData.h"
#include "WeatherRenderer.h"

class WeatherViewController : public vui::ViewController
{
public:
  virtual void loadView() override;
  
  void showWeather(WeatherData::Pointer weatherData);
  
protected:
  void unloadRenderer();
  void animateSetupVC(float w, float h);
  void renderView();
  
  vui::Button::Pointer configureButton;
  WeatherSetupViewController::Pointer setupVC;
  WeatherRenderer::Pointer weatherRenderer;
  bool weatherReady = false;
};
