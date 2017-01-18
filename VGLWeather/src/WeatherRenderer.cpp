//
//  WeatherRenderer.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/18/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#include <sstream>
#include "Renderer.h"
#include "WeatherRenderer.h"
#include "Material.h"
#include "EntityModelEditable.h"
#include "Font.h"
#include "vui/OSTextMeshGenVui.h"

using namespace std;
using namespace vom;
using namespace vgl;

WeatherRenderer::WeatherRenderer(WeatherData::Pointer weatherData, function<void()> onLoadingFinished)
  : weatherData(weatherData)
{
  string sceneFile = assetNameForWeatherCondition(weatherData->getMainDescription());
  
  //its almost always a good idea (especially on holo) to background-thread-load the scene
  //to avoid any pauses in main thread.
  scene = make_shared<Scene>(sceneFile, Scene::SLT_BACKGROUND, [=](bool success) {
    //scene has finished loading
    if(success)
    {
      time0 = System::system().globalTime();
      Renderer::getCurrent()->setSystemScene(scene.get());
      
      //for simple apps we'd be done, but we still need to generate our 3D text
      //onLoadingFinished();

      //background generate 3D text for our 3D labels
      auto font = make_shared<vui::Font>("Neuton-Regular", 5);
      System::system().scheduleBgTask([=] {
        auto model1 = make_shared<EntityModelEditable>();
        auto model2 = make_shared<EntityModelEditable>();
        auto textGen = make_shared<OSTextMeshGenVui>();
        
        ostringstream ostr;
        ostr << weatherData->getDegreesF() << "° F";
        
        model1->genText(weatherData->getLocationName(), float2::zero, nullptr, 4, font.get(), 1, true, 1, textGen.get());
        model2->genText(ostr.str(), float2::zero, nullptr, 4, font.get(), 0.75f, true, 1, textGen.get());
        
        System::system().scheduleTask([=] {
          
          auto locationMaterial = make_shared<Material>(nullptr, float4(0, 1, 1, 1), float4(0.2f, 0.2f, 0.2f, 1), float4::zero, float4::zero);
          auto temperatureMaterial = make_shared<Material>(nullptr, float4(0.5f, 1, 1, 1), float4(0.2f, 0.2f, 0.2f, 1), float4::zero, float4::zero);

          for(auto e : { model1, model2 })
          {
            QUAT q;
            get_vector_rotation_quat(&q, 90, 1, 0, 0);
            
            e->recalcBBox();
            e->updateVBOs();
            e->updateEBOs();
            e->setupVAO();
            e->setRotationQuat(q);
          }
          
          locationText = model1;
          locationText->getMeshes()[0]->setMaterial(locationMaterial);
          locationText->setPos(float3(0, 14, 30));
          
          temperatureText = model2;
          temperatureText->getMeshes()[0]->setMaterial(temperatureMaterial);
          temperatureText->setPos(float3(0, 0, 25));
          
          scene->addEntity(temperatureText);
          scene->addEntity(locationText);
          
          onLoadingFinished();
        });
      });
    }
    else
    {
      verr << "Scene load failed!" << endl;
    }
  });
}

string WeatherRenderer::assetNameForWeatherCondition(string condition)
{
  auto contains = [&condition](const string &look) {
    return (condition.find(look) != string::npos);
  };
  
  for(auto &c : condition)
    c = tolower(c);
  
  if(contains("clear") || contains("sun"))
    return "sunny.vsxproj";
  else if(contains("part") || contains("broken"))
    return "partly_cloudy.vsxproj";
  else if(contains("rain"))
    return "rain.vsxproj";
  else if(contains("storm") || contains("thunder") || contains("lightning"))
    return "storm.vsxproj";
  else if(contains("snow") || contains("winter"))
    return "snow.vsxproj";
  
  return "cloudy.vsxproj";
}

void WeatherRenderer::render()
{
  auto &system = System::system();
  auto vgl = system.systemStateMachine();
  bool holoVR = (system.getDeviceClass() == DC_HOLOGRAPHIC);
  float time = System::system().globalTime()-time0;

  vgl->pushProjection();
  vgl->pushModelView();
  if(!holoVR)
  {
    auto cam = scene->getActiveCamera();
    
    cam->applyProjection();
    cam->setTheta(90+sinf(time)*30);
    cam->setPhi(18.5f);
    cam->setDistance(187 - min(time*10, 60.0f));
  }
  
  if(time > 3)
  {
    //fade out
    auto mat = locationText->getMeshes()[0]->getMaterial();
    float4 diff = mat->getDiffuse();
    
    diff.a -= 0.05f;
    if(diff.a < 0)
    {
      diff.a = 0;
      locationText->setHidden();
    }
    mat->setDiffuse(diff);
  }
  
  scene->render();
  vgl->popModelView();
  vgl->popProjection();
  vgl->enableDepthTesting(false);
}
