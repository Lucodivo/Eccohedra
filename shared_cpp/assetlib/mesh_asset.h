#pragma once

#include "asset_loader.h"

namespace assets {
  struct Vertex_PNCV_f32 {
    f32 position[3];
    f32 normal[3];
    f32 color[3];
    f32 uv[2];
  };

  struct Vertex_P32N8C8V16 {
    f32 position[3];
    u8 normal[3];
    u8 color[3];
    f32 uv[2];
  };

  enum class VertexFormat : u32 {
    Unknown = 0,
#define VertexFormat(name) name,
#include "vertex_format.incl"
#undef VertexFormat
  };

  struct MeshBounds {
    f32 origin[3];
    f32 radius;
    f32 extents[3];
  };

  struct MeshInfo {
    u64 vertexBufferSize;
    u64 indexBufferSize;
    u8 indexSize;
    MeshBounds bounds;
    VertexFormat vertexFormat;
    CompressionMode compressionMode;
    std::string originalFile;
  };

  void readMeshInfo(const AssetFile& assetFile, MeshInfo* meshInfo);
  void unpackMesh(const MeshInfo& info, const char* srcBuffer, size_t sourceSize, char* dstVertexBuffer, char* dstIndexBuffer);
  AssetFile packMesh(const MeshInfo& meshInfo, char* vertexData, char* indexData);
  MeshBounds calculateBounds(Vertex_PNCV_f32* vertices, size_t vertexCount);
}