#include "pch.h"
#include "Renderer.h"
#include "WeatherSpaceController.h"
#include "WeatherSpace.h"
#include "WeatherSpaceContainer.h"
#include "SlateController.h"
#include "Window.h"
#include "AnimationManager.h"

using namespace std;
using namespace std::placeholders;
using namespace vui;
using namespace vui::holo;
using namespace vgl;
using namespace vom;

WeatherSpaceController::WeatherSpaceController()
{
  //root space controllers should always be set as global
  setGlobal(true);
}

void WeatherSpaceController::loadSpace()
{
  setupVC = make_shared<WeatherSetupViewController>();
  setupSC = make_shared<SlateController>(setupVC, false);
  setupSC->getSpace()->setTagAlong(true, float3::zero, 1.7f);

  //setup global container space & hook up input events so we can process them here
  auto containerSpace = make_shared<WeatherSpaceContainer>(float3::zero);
  containerSpace->onSelectBegan = bind(&WeatherSpaceController::onGlobalSpaceSelectBegan, this, _1, _2);
  containerSpace->onSelectEnded = bind(&WeatherSpaceController::onGlobalSpaceSelectEnded, this, _1, _2);
  containerSpace->onSelectMoved = bind(&WeatherSpaceController::onGlobalSpaceSelectMoved, this, _1);
  containerSpace->onSelectMovedNorm = bind(&WeatherSpaceController::onGlobalSpaceSelectMovedNorm, this, _1);

  containerSpace->addSubspace(setupSC->getSpace());

  setSpace(containerSpace);
}

void WeatherSpaceController::showWeather(WeatherData::Pointer weatherData)
{
  unloadRenderer();

  //begin loading our scene asset
  weatherRenderer = make_shared<WeatherRenderer>(weatherData, [=] {

    //now we can begin animating every frame (renderView will be called continuously henceforth)
    weatherReady = true;

    //setup weather space (which renders the 3D scene holograms)
    if(!weatherSpace)
    {
      weatherSpace = make_shared<WeatherSpace>(float3::zero);
      const float sc = 0.0024f;

      weatherSpace->setScale(float3(sc, sc, sc));
      weatherSpace->setManipulatorScaleLimits(float3(0.5f, 0.5f, 0.5f)*sc, float3(2.0f, 2.0f, 2.0f)*sc);
      weatherSpace->setNeedsPlacement(true, Renderer::getCurrent()->getSpatialSurfaceManager().get());
      weatherSpace->setSurfacePlacementOffset(float3(0, 0.0127f, 0));
      weatherSpace->onRender = bind(&WeatherSpaceController::onWeatherSpaceRender, this);
      weatherSpace->onSelectBegan = bind(&WeatherSpaceController::onWeatherSpaceSelectBegan, this, _1, _2);

      getSpace()->addSubspace(weatherSpace);
    }
    else
    {
      weatherSpace->setNeedsPlacement(true, Renderer::getCurrent()->getSpatialSurfaceManager().get());
      weatherSpace->setAnchor(nullptr);
    }

    showWeatherBoundingFrame = true;
  });

  //.. and animate the setup view controller down through the floor
  auto fromValue = make_shared<vutil::ValueWrapper>();
  auto toValue = make_shared<vutil::ValueWrapper>();
  auto setupSlate = static_pointer_cast<SlateController>(setupSC)->getSlate();

  setupSlate->setTagAlong(false);

  float3 fromPos = setupSlate->getOrigin();
  float3 toPos = setupSlate->getOrigin()-float3(0, 5, 0);

  fromValue->set<float3>(fromPos);
  toValue->set<float3>(toPos);

  auto animation = make_shared<Animation>(setupSlate, Animation::P_POSITION, fromValue, toValue, 0.8, Animation::TC_SMOOTHSTEP);
  Window::getCurrentWindow()->getAnimationManager()->addAnimation(animation);

  animation->onFinished = [=](Animation::Pointer anim) {
    setupSlate->removeFromSuperspace();
  };
}

void WeatherSpaceController::unloadRenderer()
{
  weatherReady = false;
  weatherRenderer = nullptr;
}

void WeatherSpaceController::renderSpace()
{
  //we are careful to not render the scene until it has finished loading in the background
  if(weatherReady && weatherRenderer)
  {
    if(showWeatherBoundingFrame)
    {
      if(!weatherSpace->isPlaced())
        weatherSpace->renderBoundingFrame(Space::BFT_BASIC);
      else
        weatherSpace->renderBoundingFrame(Space::BFT_TABLE_MANIP_ROT_SCALE);
    }

    weatherRenderer->render();
  }
}

void WeatherSpaceController::onGlobalSpaceSelectBegan(const float3 &pos, const ray &r)
{
  if(showWeatherBoundingFrame)
  {
    if(!weatherSpace->isPlaced())
    {
      weatherSpace->place();

      //using spatial anchors improves hologram placement stability
      weatherSpace->setAnchor(make_shared<SpatialAnchor>(weatherSpace->getOrigin(), SpatialInputHandler::currentSpatialInputHandler()));
    }
    else
    {
      weatherSpace->setAnchor(nullptr);
      weatherSpace->unplace();
    }

    if(weatherSpace->isPlaced())
    {
      auto vgl = vgl::System::system().systemStateMachine();
      vgl->setHolographicFocusPoint(weatherSpace->getOrigin());

      //weatherSpace->enableManipulatorMode(true);

      showWeatherBoundingFrame = false;
      weatherSpace->setNeedsPlacement(false, nullptr);
    }
  }
}

void WeatherSpaceController::onGlobalSpaceSelectMoved(const float3 &translation)
{

}

void WeatherSpaceController::onGlobalSpaceSelectMovedNorm(const float3 &translation)
{
}

void WeatherSpaceController::onGlobalSpaceSelectEnded(const float3 &pos, const ray &r)
{
}

void WeatherSpaceController::onWeatherSpaceSelectBegan(const float3 &pos, const ray &r)
{
  //bring back the setup UI when tapping weather hologram
  setupSC->getSpace()->setTagAlong(true, float3::zero, 1.7f);
  getSpace()->addSubspace(setupSC->getSpace());
}

void WeatherSpaceController::onWeatherSpaceRender()
{
  renderSpace();
}
