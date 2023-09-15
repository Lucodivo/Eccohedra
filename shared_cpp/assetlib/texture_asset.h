#pragma once

#include "asset_loader.h"

namespace assets {
  enum TextureFormat : u32
  {
    Unknown = 0,
#define Texture(name) name,
#include "texture_format.incl"
#undef Texture
  };

  struct TextureInfo {
    TextureFormat format;
    u32 size;
    u32 width;
    u32 height;
    std::string originalFileName;
  };

  void readTextureInfo(const AssetFile& file, TextureInfo* info);
  AssetFile packTexture(TextureInfo* info, void* data);
}
