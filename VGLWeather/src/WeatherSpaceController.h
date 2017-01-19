#pragma once

#include "SpaceController.h"
#include "WeatherSetupViewController.h"
#include "WeatherData.h"
#include "WeatherRenderer.h"
#include "WeatherSpace.h"

class WeatherSpaceController : public vui::holo::SpaceController
{
public:
  typedef std::shared_ptr<WeatherSpaceController> Pointer;
  typedef std::weak_ptr<WeatherSpaceController> WeakPointer;

  WeatherSpaceController();

  virtual void loadSpace() override;

  void showWeather(WeatherData::Pointer weatherData);

protected:
  void unloadRenderer();
  void renderSpace();

  void onGlobalSpaceSelectBegan(const float3 &pos, const vui::holo::ray &r);
  void onGlobalSpaceSelectMoved(const float3 &translation);
  void onGlobalSpaceSelectMovedNorm(const float3 &translation);
  void onGlobalSpaceSelectEnded(const float3 &pos, const vui::holo::ray &r);
  void onWeatherSpaceSelectBegan(const float3 &pos, const vui::holo::ray &r);
  void onWeatherSpaceRender();

  WeatherSpace::Pointer weatherSpace;
  SpaceController::Pointer setupSC;
  WeatherSetupViewController::Pointer setupVC;
  WeatherRenderer::Pointer weatherRenderer;
  bool weatherReady = false;
  bool showWeatherBoundingFrame = false;
};