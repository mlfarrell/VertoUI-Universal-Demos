
#include "pch.h"
#include "WeatherSpace.h"

WeatherSpace::WeatherSpace(float3 origin) : Space(origin)
{
}

Box3D WeatherSpace::getBounds()
{
  return Box3D(float3(104, 50, 104), float3(0, 0, 0));
}
