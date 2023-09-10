#pragma once

#include "asset_loader.h"

enum SkyboxFace: size_t {
  SKYBOX_FACE_FRONT = 0,
  SKYBOX_FACE_BACK,
  SKYBOX_FACE_TOP,
  SKYBOX_FACE_BOTTOM,
  SKYBOX_FACE_LEFT,
  SKYBOX_FACE_RIGHT
};

namespace assets {
  enum CubeMapFormat : u32
  {
    Unknown = 0,
#define CubeMapFormat(name, _) name,
#include "cubemap_format.incl"
#undef CubeMapFormat
  };

  const u32 cubeMapFormatSizes[]
  {
    0,
#define CubeMapFormat(_, size) size,
#include "cubemap_format.incl"
#undef CubeMapFormat
  };

  struct CubeMapInfo {
    // supplied by caller
    u64 faceSize;
    CubeMapFormat format;
    u32 faceWidth;
    u32 faceHeight;
    std::string originalFolder;

    // Filled in when packed
    CompressionMode compressionMode;

    u64 size() const { return faceSize * 6; }
    char* faceData(char* decompressedData, SkyboxFace face) const {
      return decompressedData + (face * faceSize);
    }
  };

  void readCubeMapInfo(const AssetFile& file, CubeMapInfo* info);
  void unpackCubeMap(const CubeMapInfo& texInfo, char* srcBuff, size_t srcBuffSize, char* dest);
  AssetFile packCubeMap(CubeMapInfo *info,
                        void *frontFaceData, void *backFaceData,
                        void *topFaceData, void *botFaceData,
                        void *leftFaceData, void *rightFaceData);
}
