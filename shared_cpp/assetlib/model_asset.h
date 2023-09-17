#pragma once

#include "asset_loader.h"
#include "texture_asset.h"

namespace assets {

  struct ModelDataPtrs {
    void* vertAtts;
    u64 posVertAttOffset;
    u64 normalVertAttOffset;
    u64 uvVertAttOffset;
    void* indices;
    void* albedoTex;
    void* normalTex;
  };

  struct ModelInfo {
    std::string originalFileName;

    u64 positionAttributeSize;
    u64 normalAttributeSize;
    u64 uvAttributeSize;
    u64 indicesSize;
    u32 indexTypeSize;
    u32 indexCount;
    // position num components = 3
    // normal num components = 3
    // uv num components = 2
    // type of data GL_FLOAT
    // should be normalize = false
    // stride = 0

    f32 baseColor[4];
    f32 boundingBoxMin[3];
    f32 boundingBoxDiagonal[3];

    TextureFormat normalTexFormat;
    u32 normalTexSize;
    u32 normalTexWidth;
    u32 normalTexHeight;

    TextureFormat albedoTexFormat;
    u32 albedoTexSize;
    u32 albedoTexWidth;
    u32 albedoTexHeight;

    ModelDataPtrs calcDataPts(char* data);
  };

  void readModelInfo(const AssetFile& file, ModelInfo* info);
  AssetFile packModel(ModelInfo* info,
                          void* posAttData,
                          void* normalAttData,
                          void* uvAttData,
                          void* indexData,
                          void* normalTexData,
                          void* albedoTexData);
}
