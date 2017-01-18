//
//  WeatherView.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/18/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#pragma once

#include "View.h"

class WeatherView : public vui::View
{
public:
  WeatherView(const vui::Rect &frame);
  
  virtual void render() override;
  
  //handle the rendering elsewhere
  std::function<void()> onRender;
  
protected:
};
