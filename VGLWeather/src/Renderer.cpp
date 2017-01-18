//
//  Renderer.cpp
//  VGLWeather
//
//  Created by Mike Farrell on 1/17/17.
//  Copyright © 2017 Verto Studio LLC. All rights reserved.
//

/** This class represents a basic portable VGL/VUI app shell, it should seldeom need to be modified **
**  for most basic "universal" VGL/VUI apps.                                                        **/

#include "Renderer.h"
#include "EventDispatcher.h"
#include "Window.h"
#include "Startup.h"
#include "VGLThreadScheduler.h"
#include "ClientConnection.h"

using namespace std;
using namespace vgl;
using namespace vom;

static Renderer *currentRenderer = nullptr;

Renderer::Renderer(bool holographic, float contentScale, GLuint defaultFBO)
  : isHolographic(holographic), contentScale(contentScale), defaultFramebuffer(defaultFBO)
{
  vnl::http::ThreadScheduler::setScheduler(std::make_shared<vnl::http::VGLThreadScheduler>(this));
  vnl::http::ClientConnection::setSSLRootCAPath(nullptr, true);
  
  glStateMachine = new StateMachine();
  FrameBuffer::getScreen();
  currentRenderer = this;
  
  auto textureLoader = make_shared<TextureLoaderClass>();
  Texture2D::setTextureLoader(textureLoader);

  //platform-specific setup
  //platform-specific setup
#ifndef VGLPP_HOLOGRAPHIC
  scheduleTask([this] {
    createUI();
  });
#else
  holoCursor = make_shared<vui::holo::Cursor>();
  textureLoader->setMainThreadCanBlock(false);
  
  glStateMachine->enableLight(true, 0);
  
  spatialSurfaceManager = make_shared<vui::holo::SpatialSurfaceManager>(spatialBoundingVolume);
  spatialInputHandler = make_shared<vui::holo::SpatialInputHandler>();
  
  spatialSurfaceManager->setReferenceFrame(stationaryReferenceFrame);
  spatialInputHandler->setReferenceFrame(stationaryReferenceFrame);
  spatialInputHandler->setAudioHandler(audioHandler);
  
  scheduleTask([this] {
    createUI();
  });
#endif
}

Renderer::~Renderer()
{
  scene = nullptr;
  delete glStateMachine;

#ifdef VGLPP_HOLOGRAPHIC
  spatialSurfaceManager = nullptr;
  rootSpaceController = nullptr;
#endif
}

Renderer *Renderer::getCurrent()
{
  return currentRenderer;
}

void Renderer::createUI()
{
  vglStartupUI();
}

void Renderer::setSystemScene(vom::Scene *aScene)
{
  scene = aScene;
}

#ifndef VGLPP_HOLOGRAPHIC
void Renderer::updateHoloEvents()
{
  
}
#else

void Renderer::setRootSpaceController(vui::holo::SpaceController::Pointer rootSC)
{
  rootSpaceController = rootSC;
}

void Renderer::updateHoloEvents()
{
  
  auto pos = spatialInputHandler->poll();
  if(pos)
  {
    currentInputState = pos;
    vgl->setCameraPosition(pos.headPosition);
  }
  
  holoCursor->setPos(vui::EventDispatcher::dispatcher().getHoloCursorPosition());
  holoCursor->setNormal(vui::EventDispatcher::dispatcher().getHoloCursorNormal());
  holoCursor->setState(vui::EventDispatcher::dispatcher().getHoloCursorState() ? vui::holo::Cursor::CS_ACTIVE : vui::holo::Cursor::CS_INACTIVE);
  
  spatialSurfaceManager->update();
  
  vui::holo::ray worldRay = { pos.headPosition, pos.headForwardVector };
}
#endif

bool Renderer::needsRender()
{
#ifndef VGLPP_HOLOGRAPHIC
  auto rootViewController = vui::Window::getCurrentWindow()->getRootViewController();
  return rootViewController && (rootViewController->getView()->getNeedsRender() || rootViewController->getView()->getNeedsLayout());
#else
  return true;
#endif
}

void Renderer::draw()
{
  auto vgl = glStateMachine;
  
  updateHoloEvents();
  vui::Window::getCurrentWindow()->getAnimationManager()->update();
  runScheduledTasks();
  
  vgl->enableDepthTesting(true);
  FrameBuffer::getScreen()->makeCurrent();

  FrameBuffer::getScreen()->makeCurrent();
  
  if(getDeviceClass() == DC_HOLOGRAPHIC)
  {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }
  else
  {
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  vgl->pushModelView();
  vgl->modelViewLoadIdentity();

#ifndef VGLPP_HOLOGRAPHIC
  vgl->enableDepthTesting(false);
  vgl->pushProjection();
  vgl->ortho2D(0, width/contentScale, 0, height/contentScale);
  vgl->textureMatrixLoadIdentity();
  vgl->setPrimaryColor(make_float4(1, 1, 1, 1));

  vgl->bindTexRectVAO();
  
  //vui expects initial blending state to be false
  vgl->enableBlending(false);
  vui::Layer::setBlendState(false);
  
  vgl->enableSolidTextureColorRendering(true);
  vgl->setPrimaryColor(make_float4(1, 1, 1, 1));
  vgl->prepareToDraw();
  
  //MUST be set before vui compositing
  vgl->setBlendFuncSourceFactor(vgl::BF_SRC_ALPHA, vgl::BF_ONE_MINUS_SRC_ALPHA);
  
  auto rootView = vui::Window::getCurrentWindow()->getRootViewController()->getView();
  rootView->renderLayerTreeToGL();
  
  vgl->enableSolidTextureColorRendering(false);
  
  vgl->popProjection();
#else
  auto rootSpace = vui::Window::getCurrentWindow()->getRootSpaceController()->getSpace();

  holoCursor->render();
  spatialSurfaceManager->render(true, spatialSurfaceRenderMode);
  rootSpace->renderTree();
  holoCursor->render();
#endif
  
  vgl->popModelView();
}

void Renderer::updateWindowSize(int width, int height)
{
  this->width = width;
  this->height = height;

  if(!isHolographic)
  {
    glStateMachine->setViewport(0, 0, width, height);
  }
  else
  {
#ifdef VGLPP_HOLOGRAPHIC
    int w, h;
    float scale;

    AngleHolographicGetCurrentSwapChainDimensions(&w, &h, &scale);
    glStateMachine->setViewport(0, 0, w, h);
#endif
  }
}

void Renderer::setAppSuspended(bool b)
{
  appSuspended = b;
}

uint2 Renderer::dimensionsForBackingStore()
{
  return uint2(width, height);
}

bool Renderer::isGLContextReady()
{
  return isMainThread();
}

void Renderer::bindDefaultFramebuffer()
{
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
}

float Renderer::contentScaleForBackingStore()
{
  return contentScale;
}

void Renderer::makeGLContextCurrent()
{
}

vgl::StateMachine *Renderer::systemStateMachine()
{
  VGL_ASSERT_NO_BG_THREAD();
  return glStateMachine;
}

bool Renderer::systemStateMachineExists()
{
  return (glStateMachine != nullptr);
}

bool Renderer::mainPassShouldClearFramebuffer()
{
  return true;
}

//vui::holo::SpatialInputHandler::InputState Renderer::getCurrentSpatialInputState()
//{
//  return currentInputState;
//}

vom::Scene *Renderer::systemScene()
{
  return scene;
}
