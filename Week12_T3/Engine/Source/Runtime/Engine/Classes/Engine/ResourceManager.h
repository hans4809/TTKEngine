#pragma once
#include "Texture.h"
#include "Container/Map.h"

class FGraphicsDevice;
class FResourceManager
{
public:
    void Initialize(FGraphicsDevice* device);
    void Release();

    UTexture* GetDefaultWhiteTexture();
private:
    FGraphicsDevice* GraphicDevice = nullptr;
    TMap<FWString, UTexture*> Textures;
};