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

void initCommonVertexAtt(CommonVertAtts* commonVertAtts);
void deinitCommonVertexAtts(CommonVertAtts* commonVertAtts);

void drawTriangles(const VertexAtt* vertexAtt, u32 count, u32 offset);
void drawTriangles(const VertexAtt* vertexAtt);

void deleteVertexAtt(VertexAtt* vertexAtt);
void deleteVertexAtts(VertexAtt* vertexAtts, u32 count);