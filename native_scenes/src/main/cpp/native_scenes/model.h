#pragma once

// texture ids set to TEXTURE_ID_NO_TEXTURE when none exists
// base color alpha set to 0.0 when non exists
struct TextureData {
  GLuint albedoTextureId;
  GLuint normalTextureId;
  vec4 baseColor;
};

struct Mesh {
  VertexAtt vertexAtt;
  TextureData textureData;
};

struct Model {
  Mesh* meshes;
  u32 meshCount;
  BoundingBox boundingBox;
  char* fileName;
};

void loadModelAsset(const char* filePath, Model* returnModel) {
  TimeFunction

  const u32 positionAttributeIndex = 0;
  const u32 normalAttributeIndex = 1;
  const u32 texture0AttributeIndex = 2;

  returnModel->fileName = cStrAllocateAndCopy(filePath);

  // TODO: This is NOT where exported assets directory should be stored. Move this or related solution to assetlib or potentially a asset_baker header.
  std::string bakedModelsDir = "models/";
  std::string assetPath = bakedModelsDir + filePath + ".modl";

  assets::AssetFile modelAssetFile;
  assets::loadAssetFile(assetManager_GLOBAL, assetPath.c_str(), &modelAssetFile);

  assets::ModelInfo modelInfo;
  assets::readModelInfo(modelAssetFile, &modelInfo);

  assets::ModelDataPtrs modelDataPtrs = modelInfo.calcDataPts(modelAssetFile.binaryBlob.data());

  returnModel->boundingBox.min = {modelInfo.boundingBoxMin[0],modelInfo.boundingBoxMin[1], modelInfo.boundingBoxMin[2]};
  returnModel->boundingBox.diagonal = {modelInfo.boundingBoxDiagonal[0],modelInfo.boundingBoxDiagonal[1], modelInfo.boundingBoxDiagonal[2]};

  // ==== VERTEX ATTRIBUTES ==== //
  // TODO: Handle models with more than 1 mesh
  returnModel->meshes = new Mesh[1];
  returnModel->meshCount = 1;
  Mesh& mesh = returnModel->meshes[0];
  glGenVertexArrays(1, &mesh.vertexAtt.arrayObject);
  glGenBuffers(1, &mesh.vertexAtt.bufferObject);
  glGenBuffers(1, &mesh.vertexAtt.indexObject);

  glBindVertexArray(mesh.vertexAtt.arrayObject);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexAtt.bufferObject);
  glBufferData(GL_ARRAY_BUFFER,
               modelInfo.positionAttributeSize + modelInfo.normalAttributeSize + modelInfo.uvAttributeSize,
               modelDataPtrs.vertAtts,
               GL_STATIC_DRAW);

  // set the vertex attributes (position and texture)
  // position attribute
  glVertexAttribPointer(positionAttributeIndex,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        0,// stride
                        (void*)modelDataPtrs.posVertAttOffset);
  glEnableVertexAttribArray(positionAttributeIndex);

  // normal attribute
  if(modelInfo.normalAttributeSize > 0) {
    glVertexAttribPointer(normalAttributeIndex,
                          3, // NORMAL ASSUMED TO ALWAYS BE 3-COMPONENTS
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)modelDataPtrs.normalVertAttOffset);
    glEnableVertexAttribArray(normalAttributeIndex);
  }

  // texture 0 UV Coord attribute
  if(modelInfo.uvAttributeSize > 0) {
    glVertexAttribPointer(texture0AttributeIndex,
                          2, // UV ASSUMED TO ALWAYS BE 2-COMPONENTS
                          GL_FLOAT,
                          GL_FALSE,
                          0,
                          (void*)modelDataPtrs.uvVertAttOffset);
    glEnableVertexAttribArray(texture0AttributeIndex);
  }

  // bind element buffer object to give indices
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vertexAtt.indexObject);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, modelInfo.indicesSize, modelDataPtrs.indices, GL_STATIC_DRAW);

  mesh.vertexAtt.indexCount = modelInfo.indexCount;
  mesh.vertexAtt.indexTypeSizeInBytes = modelInfo.indexTypeSize;

  // unbind VBO & VAO
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  mesh.textureData.baseColor = {modelInfo.baseColor[0], modelInfo.baseColor[1], modelInfo.baseColor[2], modelInfo.baseColor[3] };

  if(modelInfo.albedoTexSize > 0) {
    glGenTextures(1, &mesh.textureData.albedoTextureId);
    glBindTexture(GL_TEXTURE_2D, mesh.textureData.albedoTextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // disables bilinear filtering (creates sharp edges when magnifying texture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    GLenum compressedFormat = GL_INVALID_ENUM;
    if(modelInfo.albedoTexFormat == assets::TextureFormat_ETC2_RGB) { compressedFormat = GL_COMPRESSED_RGB8_ETC2; }
    else if(modelInfo.albedoTexFormat == assets::TextureFormat_ETC2_RGBA) { compressedFormat = GL_COMPRESSED_RGBA8_ETC2_EAC; }
    else { InvalidCodePath }
    glCompressedTexImage2D(GL_TEXTURE_2D,
                 0,
                 compressedFormat,
                 modelInfo.albedoTexWidth,
                 modelInfo.albedoTexHeight,
                 0,
                 modelInfo.albedoTexSize,
                 modelDataPtrs.albedoTex);
    glBindTexture(GL_TEXTURE_2D, 0);
    // TODO: Can you generate compressed mipmaps?
  } else {
    mesh.textureData.albedoTextureId = TEXTURE_ID_NO_TEXTURE;
  }

  if(modelInfo.normalTexSize > 0) {
    glGenTextures(1, &mesh.textureData.normalTextureId);
    glBindTexture(GL_TEXTURE_2D, mesh.textureData.normalTextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // disables bilinear filtering (creates sharp edges when magnifying texture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if(modelInfo.normalTexFormat == assets::TextureFormat_ETC2_RGB) {
      glCompressedTexImage2D(GL_TEXTURE_2D,
                             0,
                             GL_COMPRESSED_RGB8_ETC2,
                             modelInfo.normalTexWidth,
                             modelInfo.normalTexHeight,
                             0,
                             modelInfo.normalTexSize,
                             modelDataPtrs.normalTex);
      // TODO: Can you generate compressed mipmaps?
    } else if(modelInfo.normalTexFormat == assets::TextureFormat_RGB8) {
      glTexImage2D(GL_TEXTURE_2D,
                     0,
                   GL_RGB8,
                     modelInfo.normalTexWidth,
                     modelInfo.normalTexHeight,
                     0,
                     GL_RGB,
                   GL_UNSIGNED_BYTE,
                     modelDataPtrs.normalTex);
    } else {
      assert(false && "Unsupported texture format");
    }
    glBindTexture(GL_TEXTURE_2D, 0);
  } else {
    mesh.textureData.normalTextureId = TEXTURE_ID_NO_TEXTURE;
  }
}

void deleteModels(Model* models, u32 count) {
  std::vector<VertexAtt> vertexAtts;
  std::vector<GLuint> textureData;

  for(u32 modelIndex = 0; modelIndex < count; ++modelIndex) {
    Model* modelPtr = models + modelIndex;
    for(u32 meshIndex = 0; meshIndex < modelPtr->meshCount; ++meshIndex) {
      Mesh* meshPtr = modelPtr->meshes + meshIndex;
      vertexAtts.push_back(meshPtr->vertexAtt);
      TextureData texData = meshPtr->textureData;
      if(texData.normalTextureId != TEXTURE_ID_NO_TEXTURE) {
        textureData.push_back(texData.normalTextureId);
      }
      if(texData.albedoTextureId != TEXTURE_ID_NO_TEXTURE) {
        textureData.push_back(texData.albedoTextureId);
      }
    }
    delete[] modelPtr->meshes;
    delete[] modelPtr->fileName;
    *modelPtr = {}; // clear model to zero
  }

  deleteVertexAtts(vertexAtts.data(), (u32)vertexAtts.size());
  glDeleteTextures((GLsizei)textureData.size(), textureData.data());
}