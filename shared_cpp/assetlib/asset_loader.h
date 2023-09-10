#pragma once

// This file simply handles writing binary files for assets
// As well as a place for setting up shared structs, enums, and defines

#include <string>
#include <vector>
#include <unordered_map>

#if defined(ANDROID) || defined(__ANDROID___)
#include <android/asset_manager.h>
#endif

#include "json.hpp"
#include "lz4.h"

#include "noop_types.h"

#define FILE_TYPE_SIZE_IN_BYTES 4
#define ASSET_LIB_VERSION 1

namespace assets {
  struct AssetFile{
    char type[FILE_TYPE_SIZE_IN_BYTES];
    int version;
    std::string json; // metadata specific to asset type
    std::vector<char> binaryBlob; // the actual asset
  };

  enum class CompressionMode : u32 {
    None = 0,
#define CompressionMode(name) name,
#include "compression_mode.incl"
#undef CompressionMode
  };

#if defined(ANDROID) || defined(__ANDROID___)
  bool loadAssetFile(AAssetManager* assetManager, const char* path, AssetFile* outputFile);
#else
  bool saveAssetFile(const char* path, const AssetFile& file);
  bool loadAssetFile(const char* path, AssetFile* outputFile);
#endif

  const char* compressionModeToString(CompressionMode compressionMode);
  u32 compressionModeToEnumVal(CompressionMode compressionMode);
}
