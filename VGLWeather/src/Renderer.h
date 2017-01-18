//
//  Renderer.h
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

#pragma once

#include "vgl.h"
#include "System.h"
#include "Scene.h"
#include "ViewController.h"
#ifdef WIN10_BUILD
#include "Win10TextureLoader.h"
#define TextureLoaderClass Win10TextureLoader 
#else
#include "iOSTextureLoader.h"
#define TextureLoaderClass iOSTextureLoader
#endif

class Renderer : public vgl::System
{
public:
  Renderer(bool holographic, float contentScale, GLuint defaultFBO);
  virtual ~Renderer();
  
  static Renderer *getCurrent();
  
  void createUI();
  
  void setAppSuspended(bool b);
  inline bool isAppSuspended() { return appSuspended.load(); }
  
  void updateHoloEvents();
  bool needsRender();
  void draw();
  void updateWindowSize(int width, int height);

  virtual uint2 dimensionsForBackingStore() override;
  
  ///Return true if the OpenGL context is up and running
  virtual bool isGLContextReady() override;
  
  ///This method should bind the default framebuffer to the context
  ///on desktop systems this is usually framebuffer 0
  virtual void bindDefaultFramebuffer() override;
  
  void setSystemScene(vom::Scene *scene);
  
  ///Retina\Hi-DPI display scale
  virtual float contentScaleForBackingStore() override;
  
  ///Makes the current GL context current, you must call superclass method
  ///when implementing this method.
  virtual void makeGLContextCurrent() override;
  
  ///returns access to the current state machine
  virtual vgl::StateMachine *systemStateMachine() override;
  
  virtual bool systemStateMachineExists() override;
  
  ///Return true if production rendering of main-pass should clear the framebuffer to the
  ///VGL background color
  virtual bool mainPassShouldClearFramebuffer() override;
  
  ///Some systems have a shared global system scene accessible from anywhere at all times
  ///Provide that here to activate it (and retrieve via Scene::systemScene())
  virtual vom::Scene *systemScene() override;
    
protected:
  int width, height;
  bool isHolographic;
  float contentScale = 1.0f;

  GLuint defaultFramebuffer = 0;
  vgl::StateMachine *glStateMachine = nullptr;
  vom::Scene *scene = nullptr;
  
  std::atomic_bool appSuspended;

#ifdef VGLPP_HOLOGRAPHIC
  Windows::Perception::Spatial::SpatialStationaryFrameOfReference ^stationaryReferenceFrame = nullptr;
  
  vui::holo::SpatialInputHandler::Pointer spatialInputHandler;
  vui::holo::SpatialSurfaceManager::Pointer spatialSurfaceManager;
  
  vui::holo::Cursor::Pointer holoCursor;
  vui::holo::SpatialInputHandler::InputState currentInputState;
#endif
};
