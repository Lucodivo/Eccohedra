#pragma once

#include "asset_loader.h"

enum SkyboxFace: size_t {
  SKYBOX_FACE_FRONT = 0,
  SKYBOX_FACE_BACK,
  SKYBOX_FACE_TOP,
  SKYBOX_FACE_BOTTOM,
  SKYBOX_FACE_RIGHT,
  SKYBOX_FACE_LEFT,
};

namespace assets {
  enum CubeMapFormat : u32
  {
    Unknown = 0,
#define CubeMapFormat(name) name,
#include "cubemap_format.incl"
#undef CubeMapFormat
  };

  struct CubeMapInfo {
    CubeMapFormat format;
    u32 faceSize;
    u32 faceWidth;
    u32 faceHeight;
    std::string originalFolder;

    u64 size() const { return faceSize * 6; }
    char* faceData(char* data, SkyboxFace face) const { return data + (face * faceSize); }
  };

  void readCubeMapInfo(const AssetFile& file, CubeMapInfo* info);
  void unpackCubeMap(const CubeMapInfo& texInfo, char* srcBuff, size_t srcBuffSize, char* dest);
  AssetFile packCubeMap(CubeMapInfo *info, void* data_FBTBLR);
}
