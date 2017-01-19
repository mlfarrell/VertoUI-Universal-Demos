
#pragma once

#include "Space.h"

class WeatherSpace : public vui::holo::Space
{
public:
  typedef std::shared_ptr<WeatherSpace> Pointer;
  typedef std::weak_ptr<WeatherSpace> WeakPointer;

  WeatherSpace(float3 origin);

  virtual Box3D getBounds() override;

  virtual void selectBegan(const float3 &pos, const vui::holo::ray &r) override
  {
    Space::selectBegan(pos, r);

    if(onSelectBegan)
      onSelectBegan(pos, r);
  }

  virtual void selectMovedNorm(const float3 &normalizedOffset) override
  {
    Space::selectMovedNorm(normalizedOffset);

    if(onSelectMovedNorm)
      onSelectMovedNorm(normalizedOffset);
  }

  virtual void selectMoved(const float3 &translation) override
  {
    Space::selectMoved(translation);

    if(onSelectMoved)
      onSelectMoved(translation);
  }

  virtual void selectEnded(const float3 &pos, const vui::holo::ray &r) override
  {
    Space::selectEnded(pos, r);

    if(onSelectEnded)
      onSelectEnded(pos, r);
  }

  virtual void render() override
  {
    Space::render();

    onRender();
  }

  std::function<void()> onRender;
  std::function<void(const float3 &pos, const vui::holo::ray &r)> onSelectBegan, onSelectEnded;
  std::function<void(const float3 &translation)> onSelectMovedNorm, onSelectMoved;
};
