#pragma once

#include <string>

#include "pch.h"
#include "Renderer.h"

namespace VGLWeather
{
    ref class App sealed : public Windows::ApplicationModel::Core::IFrameworkView
    {
    public:
        App();

        void makeContextCurrent();

        // IFrameworkView Methods.
        virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
        virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
        virtual void Load(Platform::String^ entryPoint);
        virtual void Run();
        virtual void Uninitialize();

        static App ^getCurrent();

    internal:
      inline Renderer *getRenderer() { return mRenderer.get(); }


    private:
        void RecreateRenderer();

        // Application lifecycle event handlers.
        void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);

        // Window event handlers.
        void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
        void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

        void OnLocatabilityChanged(Windows::Perception::Spatial::SpatialLocator ^locator, Platform::Object ^args);

        void InitializeEGL(Windows::Graphics::Holographic::HolographicSpace^ holographicSpace);
        void InitializeEGL(Windows::UI::Core::CoreWindow^ window);
        void App::InitializeEGLInner(Platform::Object^ windowBasis);
        void CleanupEGL();

        bool mWindowClosed;
        bool mWindowVisible;
        
        EGLDisplay mEglDisplay;
        EGLContext mEglContext;
        EGLSurface mEglSurface;

        std::unique_ptr<Renderer> mRenderer;

        Windows::Graphics::Holographic::HolographicSpace ^mHolographicSpace = nullptr;
        Windows::Perception::Spatial::SpatialLocator ^mLocator = nullptr;
        Windows::Perception::Spatial::SpatialStationaryFrameOfReference ^mStationaryReferenceFrame = nullptr;
    };

}