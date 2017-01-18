//
//  WeatherViewController.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#include "System.h"
#include "Window.h"
#include "WeatherViewController.h"
#include "WeatherView.h"
#include "WeatherSetupViewController.h"
#include "Colors.h"
#include "Animation.h"

using namespace std;
using namespace vui;

void WeatherViewController::loadView()
{
  auto fullSize = vgl::System::system().dimensionsForBackingStore();
  float2 fullSzFloat = float2(fullSize.x, fullSize.y);
  
  fullSzFloat *= 1.0f/(vgl::System::system().contentScaleForBackingStore());
  
  auto view = make_shared<WeatherView>(Rect(0, 0, fullSzFloat.x, fullSzFloat.y));
  view->onRender = bind(&WeatherViewController::renderView, this);
  view->setBackgroundColor(Colors::DarkGray);
    
  auto setupViewController = make_shared<WeatherSetupViewController>();
  auto setupView = setupViewController->getView();
  view->addSubview(setupView);
  setupVC = setupViewController;
  
  configureButton = make_shared<Button>(Rect(10, 10, 100, 40), "Configure");
  configureButton->setTextColor(float4(0.5f, 1, 1, 1));
  configureButton->onPress = [this, fullSzFloat](Control::Pointer sender) { //always watch for retain cycles when using lambda event handlers
    
    getView()->addSubview(setupVC->getView());
    animateSetupVC(fullSzFloat.x, fullSzFloat.y);
  };
  configureButton->setHidden(true);
  view->addSubview(configureButton);
  
  animateSetupVC(fullSzFloat.x, fullSzFloat.y);
  
  setView(view);
}

void WeatherViewController::animateSetupVC(float w, float h)
{
  //animate the setup view controller onto the screen
  auto fromValue = make_shared<vutil::ValueWrapper>();
  auto toValue = make_shared<vutil::ValueWrapper>();
  auto setupViewController = setupVC;
  auto setupView = setupViewController->getView();
  
  float2 fromPos = float2(w/2-setupView->getFrame().w/2, h/2-setupView->getFrame().h/2-h);
  float2 toPos = float2(w/2-setupView->getFrame().w/2, h/2-setupView->getFrame().h/2+50);

  setupView->setPos(fromPos);
  fromValue->set<float2>(fromPos);
  toValue->set<float2>(toPos);
  
  auto animation = make_shared<Animation>(setupView, Animation::P_POSITION, fromValue, toValue, 0.8, Animation::TC_SMOOTHSTEP);
  Window::getCurrentWindow()->getAnimationManager()->addAnimation(animation);

  animation->onFinished = [=](Animation::Pointer anim) {
    configureButton->setHidden(true);
    unloadRenderer();
  };
}

void WeatherViewController::renderView()
{
  //we are careful to not render the scene until it has finished loading in the background
  if(weatherReady && weatherRenderer)
  {
    weatherRenderer->render();
  }
}

void WeatherViewController::showWeather(WeatherData::Pointer weatherData)
{
  //begin loading our scene asset
  weatherRenderer = make_shared<WeatherRenderer>(weatherData, [=] {
    
    //now we can begin animating every frame (renderView will be called continuously henceforth)
    Window::getCurrentWindow()->getAnimationManager()->setAnimationOn(true);
    weatherReady = true;
    configureButton->setHidden(false);
  });
  
  //.. and animate the setup view controller off of the screen
  auto fromValue = make_shared<vutil::ValueWrapper>();
  auto toValue = make_shared<vutil::ValueWrapper>();
  auto setupViewController = static_pointer_cast<WeatherSetupViewController>(setupVC);
  auto setupView = setupViewController->getView();
  
  Rect frame = getView()->getFrame();
  float w = frame.w, h = frame.h;
  float2 fromPos = float2(w/2-setupView->getFrame().w/2, h/2-setupView->getFrame().h/2+50);
  float2 toPos = float2(w/2-setupView->getFrame().w/2, h/2-setupView->getFrame().h/2-h);
  
  setupView->setPos(fromPos);
  fromValue->set<float2>(fromPos);
  toValue->set<float2>(toPos);
  
  auto animation = make_shared<Animation>(setupView, Animation::P_POSITION, fromValue, toValue, 0.8, Animation::TC_SMOOTHSTEP);
  Window::getCurrentWindow()->getAnimationManager()->addAnimation(animation);
  
  animation->onFinished = [=](Animation::Pointer anim) {
    setupView->removeFromSuperview();
  };
}

void WeatherViewController::unloadRenderer()
{
  weatherReady = false;
  weatherRenderer = nullptr;
  Window::getCurrentWindow()->getAnimationManager()->setAnimationOn(false);
}

