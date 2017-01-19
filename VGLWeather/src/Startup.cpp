//
//  Startup.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#include "pch.h"
#include "Startup.h"
#include "System.h"
#include "StateMachine.h"
#include "Window.h"
#include "WeatherViewController.h"
#include "WeatherSpaceController.h"

using namespace std;

void vglStartupUI()
{
  //load your initial UI here
  if(vgl::System::system().getDeviceClass() != vgl::DC_HOLOGRAPHIC)
  {
    auto weatherVC = make_shared<WeatherViewController>();
    vui::Window::getCurrentWindow()->setRootViewController(weatherVC);
    
    //using a direct layer for the fullscreen root VC provides a much-needed optimization
    //in the compositor on mobile hardware
    weatherVC->getView()->getLayer()->setDirect(true);
  }
  else
  {
    auto weatherSC = make_shared<WeatherSpaceController>();
    vui::Window::getCurrentWindow()->setRootSpaceController(weatherSC);
  }
  
  //OpenGL cannot background-thread compile shaders
  //prime any system shaders here that you may need, this avoids pauses in the main thread later
  vgl::System::system().systemStateMachine()->systemShaderEffect("Cartoon");
}
