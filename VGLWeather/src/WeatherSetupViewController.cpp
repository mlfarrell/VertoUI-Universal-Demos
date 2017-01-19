//
//  WeatherSetupViewController.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#include "pch.h"
#include "WeatherSetupViewController.h"
#include "WeatherViewController.h"
#include "Window.h"
#include "AlertBoxController.h"
#include "Colors.h"
#include "ImageView.h"
#include "Button.h"
#include "Label.h"
#include "TextEntry.h"
#include "ClientConnection.h"
#include "json.hpp"
#ifdef VGLPP_HOLOGRAPHIC
#include "WeatherSpaceController.h"
#endif

using json = nlohmann::json;
using namespace std;
using namespace vui;

void WeatherSetupViewController::loadView()
{
  auto view = make_shared<View>(Rect(0, 0, 512, 384));
  view->setBackgroundColor(Colors::White);
  
  auto image = make_shared<Image>("weather.png");
  auto imageView = make_shared<ImageView>(image);
  imageView->setFrame(Rect(0, 0, 512, 384));
  view->addSubview(imageView);
  
  auto label = make_shared<Label>("Enter your location: ");
  label->setPos(float2(15+65, 384/2-label->getFrame().h/2));
  label->setTextColor(Colors::Black);
  view->addSubview(label);  

  auto field = make_shared<TextEntry>(Rect(0, 0, 150, 40), "92109");
  field->setPos(float2(180+65, 384/2-field->getFrame().h/2));
  view->addSubview(field);
  zipCodeEntry = field;

  auto button = make_shared<Button>(Rect(0, 0, 80, 40), "Go!");
  button->setPos(float2(512-80-10, 10));
  button->setBackgroundColor(float4(0.2f, 0.2f, 0.4f, 1.0f));
  button->onPress = bind(&WeatherSetupViewController::onGoPressed, this, placeholders::_1);
  view->addSubview(button);

  setView(view);
}

void WeatherSetupViewController::showWeather()
{
#ifndef VGLPP_HOLOGRAPHIC
  auto rootVC = dynamic_pointer_cast<WeatherViewController>(Window::getCurrentWindow()->getRootViewController());
  rootVC->showWeather(weatherData);
#else
  auto rootSC = dynamic_pointer_cast<WeatherSpaceController>(Window::getCurrentWindow()->getRootSpaceController());
  rootSC->showWeather(weatherData);
#endif
}

void WeatherSetupViewController::onGoPressed(Control::Pointer sender)
{
  using namespace vnl::http;
  
  auto isZipCode = [](const string &str) {
    for(auto c : str)
    {
      if(isdigit(c))
        return true;
    }
    
    return false;
  };
  
  sender->setEnabled(false);
  busy = true;
  
  string baseURL = "api.openweathermap.org";
  string apiURLString = "/data/2.5/weather";
  string locationString = zipCodeEntry->getText();
  Request request(baseURL + apiURLString);
  
  if(isZipCode(locationString))
    request.appendQuery("zip", locationString+",us");
  else
    request.appendQuery("q", locationString+",us");
  request.appendQuery("APPID", "9b1362a0a5194412b60e4f87487694d7");

  json j =
  {
    { "operation", "sessioninfo" },
  };
  auto conn = make_shared<ClientConnection>(request, ClientConnection::GET, j.dump());
  
  auto self = static_pointer_cast<WeatherSetupViewController>(shared_from_this());
  conn->onSucceeded = [self, sender](ClientConnection *conn) {
    auto jObj = json::parse(conn->getResponseText());
    
    vgl::System::system().scheduleTask([self, sender, jObj] {

      vout << jObj.dump() << endl;
      
      try
      {
        if(jObj != nullptr && (int)jObj["cod"] == 200)
        {
          auto kelvinToF = [](float k) {
            return (int)roundf(k * 9/5.0f - 459.67f);
          };
          
          string descrption = jObj["weather"][0]["description"];
          string mainDescription = jObj["weather"][0]["main"];
          int degreesF = kelvinToF((int)jObj["main"]["temp"]);
          
          if(descrption.find("broken clouds") != string::npos)
          {
            mainDescription = "broken clouds";
          }
          
          self->weatherData = make_shared<WeatherData>(jObj["name"], mainDescription, degreesF);
          self->showWeather();
        }
        else
        {
          vector<string> titles = { "Ok" };
          auto alert = make_shared<AlertBoxController>("Request Error", "Something went wrong during the Weather Request Operation", titles);
          ViewController::Pointer vc = self;
          
          if(vgl::System::system().getDeviceClass() != vgl::DC_HOLOGRAPHIC)
          {
            vc = Window::getCurrentWindow()->getRootViewController();
            alert->setOrigin({ vc->getView()->getFrame().w/2-alert->getView()->getFrame().w/2, vc->getView()->getFrame().h/2-alert->getView()->getFrame().h/2 },
                             nullptr, vui::ViewController::OA_TOP_LEFT);
          }
          else
          {
            alert->setOrigin(float2::zero, sender, vui::ViewController::OA_TOP_LEFT);
          }
          
          vc->pushModalViewController(alert);
        }
      }
      catch(...)
      {
        verr << "exception" << endl;
      }
      
      self->busy = false;
      sender->setEnabled(true);
    });
  };
  
  conn->onFailed = [self](ClientConnection *conn) {
    verr << "error" << endl;
  };
  
  //conn->setCookiesPath(cookiesPath);
  conn->start();
}

