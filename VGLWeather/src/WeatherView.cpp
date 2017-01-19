//
//  WeatherView.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/18/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#include "pch.h"
#include "WeatherView.h"

WeatherView::WeatherView(const vui::Rect &frame) : View(frame)
{
  
}

void WeatherView::render()
{
  View::render();
  
  onRender();
}
