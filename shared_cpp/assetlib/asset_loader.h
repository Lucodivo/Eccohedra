#pragma once

// This file simply handles writing binary files for assets
// As well as a place for setting up shared structs, enums, and defines

#include <string>
#include <vector>
#include <unordered_map>

#include "json.hpp"
#include "lz4.h"

#include "noop_types.h"

#if defined(ANDROID) || defined(__ANDROID___)
#include "android_platform.h"
#else
#define LOGI(...) printf(__VA_ARGS__)
#define LOGW(...) printf(__VA_ARGS__)
#define LOGE(...) printf(__VA_ARGS__)
#endif

#define FILE_TYPE_SIZE_IN_BYTES 4
#define ASSET_LIB_VERSION 1

namespace assets {
  struct AssetFile{
    char type[FILE_TYPE_SIZE_IN_BYTES];
    u32 version;
    // TODO: Remove json, just use bytes
    std::string json; // metadata specific to asset type
    std::vector<char> binaryBlob; // the actual asset
  };

#if defined(ANDROID) || defined(__ANDROID___)
  bool loadAssetFile(AAssetManager* assetManager, const char* path, AssetFile* outputFile);
#else
  bool saveAssetFile(const char* path, const AssetFile& file);
  bool loadAssetFile(const char* path, AssetFile* outputFile);
#endif
}
