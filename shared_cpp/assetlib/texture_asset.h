#pragma once

#include "asset_loader.h"

namespace assets {
  enum class TextureFormat : u32
  {
    Unknown = 0,
#define TextureFormat(name) name,
#include "texture_format.incl"
#undef TextureFormat
  };

  struct TextureInfo {
    // supplied by caller
    u64 textureSize;
    TextureFormat textureFormat;
    u32 width;
    u32 height;
    std::string originalFile;

    // Filled in when packed
    CompressionMode compressionMode;
  };

  //parses the texture metadata from an asset file
  void readTextureInfo(const AssetFile& file, TextureInfo* texInfo);

  void unpackTexture(const TextureInfo& texInfo, const char* sourceBuffer, size_t sourceSize, char* destination);

  AssetFile packTexture(TextureInfo* info, void* pixelData);
}
