#pragma once

struct VertexAtt {
  GLuint arrayObject;
  GLuint bufferObject;
  GLuint indexObject;
  u32 indexCount;
  u32 indexTypeSizeInBytes;
};

struct CommonVertAtts {
  union {
    struct {
      VertexAtt cubePos;
      VertexAtt invertedCubePos;
      VertexAtt cubePos_OpenNegYFace;
      VertexAtt invertedCubePos_OpenNegYFace;
      VertexAtt quadPos;
      VertexAtt quadPosTex;
    };
    VertexAtt array[6];
  };

  VertexAtt* cube(bool invertedWindingOrder = false, bool openNegYFace = false);
  VertexAtt* quad(bool textureAtt = false);
};

const vec3 cubeFaceNegativeXCenter{-0.5f, 0.0f, 0.0f};
const vec3 cubeFacePositiveXCenter{0.5f, 0.0f, 0.0f};
const vec3 cubeFaceNegativeYCenter{0.0f, -0.5f, 0.0f};
const vec3 cubeFacePositiveYCenter{0.0f, 0.5f, 0.0f};
const vec3 cubeFaceNegativeZCenter{0.0f, 0.0f, -0.5f};
const vec3 cubeFacePositiveZCenter{0.0f, 0.0f, 0.5f};
// center values are simply doubled to get a normalized vector
const vec3 negativeXNormal = vec3{-1.0f, 0.0f, 0.0f};
const vec3 positiveXNormal = vec3{1.0f, 0.0f, 0.0f};
const vec3 negativeYNormal = vec3{0.0f, -1.0f, 0.0f};
const vec3 positiveYNormal = vec3{0.0f, 1.0f, 0.0f};
const vec3 negativeZNormal = vec3{0.0f, 0.0f, -1.0f};
const vec3 positiveZNormal = vec3{0.0f, 0.0f, 1.0f};
const BoundingBox cubeVertAttBoundingBox = {
        {-0.5, -0.5, -0.5},
        {1.0f, 1.0f, 1.0f}
};

void initCommonVertexAtt(CommonVertAtts* commonVertAtts);
void deinitCommonVertexAtts(CommonVertAtts* commonVertAtts);

void drawTriangles(const VertexAtt* vertexAtt, u32 count, u32 offset);
void drawTriangles(const VertexAtt* vertexAtt);

void deleteVertexAtt(VertexAtt* vertexAtt);
void deleteVertexAtts(VertexAtt* vertexAtts, u32 count);