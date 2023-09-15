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

  // TODO: Merge CubeMap format with TextureFormat?
  enum CubeMapFormat : u32
  {
    Unknown = 0,
#define Texture(name) name,
#include "texture_format.incl"
#undef Texture
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
  AssetFile packCubeMap(CubeMapInfo *info, void* data_FBTBLR);
}
