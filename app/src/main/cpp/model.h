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
};

b32 baseColorValid(const TextureData& textureData) {
  return textureData.baseColor.w != 0.0f;
}

void loadModelTexture(u32* textureId, tinygltf::Image* image, b32 inputSRGB = false)
{
  glGenTextures(1, textureId);
  glBindTexture(GL_TEXTURE_2D, *textureId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // disables bilinear filtering (creates sharp edges when magnifying texture)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  u8* imageData = image->image.data();
  u32 numComponents = image->component;

  // load image data
  if (!image->image.empty() && numComponents <= 4)
  {
    u32 dataColorSpace;
    u32 dataComponentComposition;
    switch(numComponents) {
      case 1:
        dataColorSpace = dataComponentComposition = GL_RED;
        break;
      case 2:
        dataColorSpace = dataComponentComposition = GL_RG;
        break;
      case 3:
        dataColorSpace = inputSRGB ? GL_SRGB : GL_RGB;
        dataComponentComposition = GL_RGB;
        break;
      case 4:
        dataColorSpace = inputSRGB ? GL_SRGB8_ALPHA8 : GL_RGBA;
        dataComponentComposition = GL_RGBA;
        break;
      default:
        InvalidCodePath;
    }

    glTexImage2D(GL_TEXTURE_2D, // target
                 0, // level of detail (level n is the nth mipmap reduction image)
                 dataColorSpace, // What is the color space of the data
                 image->width, // width of texture
                 image->height, // height of texture
                 0, // border (legacy stuff, MUST BE 0)
                 dataComponentComposition, // How are the components of the data composed
                 GL_UNSIGNED_BYTE, // specifies data type of pixel data
                 imageData); // pointer to the image data
    glGenerateMipmap(GL_TEXTURE_2D);

    // set texture options
  } else
  {
    LOGW("Failed to load texture");
  }

  glBindTexture(GL_TEXTURE_2D, 0);
}

void initializeModelVertexData(tinygltf::Model* gltfModel, Model* model)
{
  struct gltfAttributeMetadata {
    u32 accessorIndex;
    u32 numComponents;
    u32 bufferViewIndex;
    u32 bufferIndex;
    u64 bufferByteOffset;
    u64 bufferByteLength;
  };

  const char* positionIndexKeyString = "POSITION";
  const char* normalIndexKeyString = "NORMAL";
  const char* texture0IndexKeyString = "TEXCOORD_0";

  model->meshCount = (u32)gltfModel->meshes.size();
  Assert(model->meshCount != 0);
  model->meshes = new Mesh[model->meshCount];
  std::vector<tinygltf::Accessor>* gltfAccessors = &gltfModel->accessors;
  std::vector<tinygltf::BufferView>* gltfBufferViews = &gltfModel->bufferViews;

  auto populateAttributeMetadata = [gltfAccessors, gltfBufferViews](const char* keyString, const tinygltf::Primitive& gltfPrimitive) -> gltfAttributeMetadata {
    gltfAttributeMetadata result;
    result.accessorIndex = gltfPrimitive.attributes.at(keyString);
    result.numComponents = tinygltf::GetNumComponentsInType(gltfAccessors->at(result.accessorIndex).type);
    result.bufferViewIndex = gltfAccessors->at(result.accessorIndex).bufferView;
    result.bufferIndex = gltfBufferViews->at(result.bufferViewIndex).buffer;
    result.bufferByteOffset = gltfBufferViews->at(result.bufferViewIndex).byteOffset;
    result.bufferByteLength = gltfBufferViews->at(result.bufferViewIndex).byteLength;
    return result;
  };

  for(u32 i = 0; i < model->meshCount; ++i) {
    Mesh* mesh = &model->meshes[i];

    tinygltf::Mesh gltfMesh = gltfModel->meshes[i];
    Assert(!gltfMesh.primitives.empty());
    // TODO: handle meshes that have more than one primitive
    tinygltf::Primitive gltfPrimitive = gltfMesh.primitives[0];
    Assert(gltfPrimitive.indices > -1); // TODO: Should we deal with models that don't have indices?

    // TODO: Allow variability in attributes beyond POSITION, NORMAL, TEXCOORD_0?
    Assert(gltfPrimitive.attributes.find(positionIndexKeyString) != gltfPrimitive.attributes.end());
    gltfAttributeMetadata positionAttribute = populateAttributeMetadata(positionIndexKeyString, gltfPrimitive);
    f64* minValues = gltfModel->accessors[positionAttribute.accessorIndex].minValues.data();
    f64* maxValues = gltfModel->accessors[positionAttribute.accessorIndex].maxValues.data();
    model->boundingBox.min = {(f32)minValues[0], (f32)minValues[1], (f32)minValues[2]};
    model->boundingBox.diagonal = vec3{(f32)maxValues[0], (f32)maxValues[1], (f32)maxValues[2]} - model->boundingBox.min;

    b32 normalAttributesAvailable = gltfPrimitive.attributes.find(normalIndexKeyString) != gltfPrimitive.attributes.end();
    gltfAttributeMetadata normalAttribute{};
    if(normalAttributesAvailable) { // normal attribute data
      normalAttribute = populateAttributeMetadata(normalIndexKeyString, gltfPrimitive);
      Assert(positionAttribute.bufferIndex == normalAttribute.bufferIndex);
    }

    b32 texture0AttributesAvailable = gltfPrimitive.attributes.find(texture0IndexKeyString) != gltfPrimitive.attributes.end();
    gltfAttributeMetadata texture0Attribute{};
    if(texture0AttributesAvailable) { // texture 0 uv coord attribute data
      texture0Attribute = populateAttributeMetadata(texture0IndexKeyString, gltfPrimitive);
      Assert(positionAttribute.bufferIndex == texture0Attribute.bufferIndex);
    }

    // TODO: Handle vertex attributes that don't share the same buffer?
    u32 vertexAttBufferIndex = positionAttribute.bufferIndex;
    Assert(gltfModel->buffers.size() > vertexAttBufferIndex);

    u32 indicesAccessorIndex = gltfPrimitive.indices;
    tinygltf::BufferView indicesGLTFBufferView = gltfBufferViews->at(gltfAccessors->at(indicesAccessorIndex).bufferView);
    u32 indicesGLTFBufferIndex = indicesGLTFBufferView.buffer;
    u64 indicesGLTFBufferByteOffset = indicesGLTFBufferView.byteOffset;
    u64 indicesGLTFBufferByteLength = indicesGLTFBufferView.byteLength;

    u64 minOffset = Min(positionAttribute.bufferByteOffset, Min(texture0Attribute.bufferByteOffset, normalAttribute.bufferByteOffset));
    u8* vertexAttributeDataOffset = gltfModel->buffers[indicesGLTFBufferIndex].data.data() + minOffset;
    u8* indicesDataOffset = gltfModel->buffers[indicesGLTFBufferIndex].data.data() + indicesGLTFBufferByteOffset;

    mesh->vertexAtt.indexCount = u32(gltfAccessors->at(indicesAccessorIndex).count);
    mesh->vertexAtt.indexTypeSizeInBytes = tinygltf::GetComponentSizeInBytes(gltfAccessors->at(indicesAccessorIndex).componentType);
    // TODO: Handle the possibility of the three attributes not being side-by-side in the buffer
    u64 sizeOfAttributeData = positionAttribute.bufferByteLength + normalAttribute.bufferByteLength + texture0Attribute.bufferByteLength;
    Assert(gltfModel->buffers[vertexAttBufferIndex].data.size() >= sizeOfAttributeData);
    const u32 positionAttributeIndex = 0;
    const u32 normalAttributeIndex = 1;
    const u32 texture0AttributeIndex = 2;

    glGenVertexArrays(1, &mesh->vertexAtt.arrayObject);
    glGenBuffers(1, &mesh->vertexAtt.bufferObject);
    glGenBuffers(1, &mesh->vertexAtt.indexObject);

    glBindVertexArray(mesh->vertexAtt.arrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertexAtt.bufferObject);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeOfAttributeData,
                 vertexAttributeDataOffset,
                 GL_STATIC_DRAW);

    // set the vertex attributes (position and texture)
    // position attribute
    glVertexAttribPointer(positionAttributeIndex,
                          positionAttribute.numComponents, // attribute size
                          GL_FLOAT, // type of data
                          GL_FALSE, // should data be normalized
                          positionAttribute.numComponents * sizeof(f32),// stride
                          (void*)(positionAttribute.bufferByteOffset - minOffset)); // offset of first component
    glEnableVertexAttribArray(positionAttributeIndex);

    // normal attribute
    if(normalAttributesAvailable) {
      glVertexAttribPointer(normalAttributeIndex,
                            normalAttribute.numComponents, // attribute size
                            GL_FLOAT,
                            GL_FALSE,
                            normalAttribute.numComponents * sizeof(f32),
                            (void*)(normalAttribute.bufferByteOffset - minOffset));
      glEnableVertexAttribArray(normalAttributeIndex);
    }

    // texture 0 UV Coord attribute
    if(texture0AttributesAvailable) {
      glVertexAttribPointer(texture0AttributeIndex,
                            texture0Attribute.numComponents, // attribute size
                            GL_FLOAT,
                            GL_FALSE,
                            texture0Attribute.numComponents * sizeof(f32),
                            (void*)(texture0Attribute.bufferByteOffset - minOffset));
      glEnableVertexAttribArray(texture0AttributeIndex);
    }

    // bind element buffer object to give indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->vertexAtt.indexObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesGLTFBufferByteLength, indicesDataOffset, GL_STATIC_DRAW);

    // unbind VBO & VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    s32 gltfMaterialIndex = gltfPrimitive.material;
    if(gltfMaterialIndex >= 0) {
      tinygltf::Material gltfMaterial = gltfModel->materials[gltfMaterialIndex];
      // TODO: Handle more then just TEXCOORD_0 vertex attribute?
      Assert(gltfMaterial.normalTexture.texCoord == 0 && gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord == 0);

      f64* baseColor = gltfMaterial.pbrMetallicRoughness.baseColorFactor.data();
      mesh->textureData.baseColor = {(f32)baseColor[0], (f32)baseColor[1], (f32)baseColor[2], (f32)baseColor[3] };

      // NOTE: gltf.textures.samplers gives info about how to magnify/minify textures and how texture wrapping should work
      // TODO: Don't load the same texture multiple times if multiple meshes use the same texture
      s32 normalTextureIndex = gltfMaterial.normalTexture.index;
      if(normalTextureIndex >= 0) {
        u32 normalImageIndex = gltfModel->textures[normalTextureIndex].source;
        tinygltf::Image normalImage = gltfModel->images[normalImageIndex];
        loadModelTexture(&mesh->textureData.normalTextureId, &normalImage);
      } else {
        mesh->textureData.normalTextureId = TEXTURE_ID_NO_TEXTURE;
      }

      s32 baseColorTextureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
      if(baseColorTextureIndex >= 0) {
        u32 albedoColorImageIndex = gltfModel->textures[baseColorTextureIndex].source;
        tinygltf::Image albedoImage = gltfModel->images[albedoColorImageIndex];
        loadModelTexture(&mesh->textureData.albedoTextureId, &albedoImage);
      } else {
        mesh->textureData.albedoTextureId = TEXTURE_ID_NO_TEXTURE;
      }
    } else {
      mesh->textureData.normalTextureId = TEXTURE_ID_NO_TEXTURE;
      mesh->textureData.albedoTextureId = TEXTURE_ID_NO_TEXTURE;
      mesh->textureData.baseColor = {};
    }
  }
}

void loadModel(const void* fileBuffer, const u32 sizeInBytes, Model* returnModel) {
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  tinygltf::Model tinyGLTFModel;

  bool ret = loader.LoadBinaryFromMemory(&tinyGLTFModel, &err, &warn, (const unsigned char*)fileBuffer, sizeInBytes);

  if (!warn.empty()) {
    printf("Warning: %s\n", warn.c_str());
    return;
  }

  if (!err.empty()) {
    printf("Error: %s\n", err.c_str());
    return;
  }

  if (!ret) {
    printf("Failed to parse glTF\n");
    return;
  }

  initializeModelVertexData(&tinyGLTFModel, returnModel);
}

void loadModels(const char** filePaths, u32 count, Model** returnModels) {
  tinygltf::TinyGLTF loader;

  for(u32 i = 0; i < count; i++) {
    std::string err;
    std::string warn;
    tinygltf::Model tinyGLTFModel;

    bool ret = loader.LoadBinaryFromFile(&tinyGLTFModel, &err, &warn, filePaths[i]); // for binary glTF(.glb)

    if (!warn.empty()) {
      printf("Warn: %s\n", warn.c_str());
      return;
    }

    if (!err.empty()) {
      printf("Err: %s\n", err.c_str());
      return;
    }

    if (!ret) {
      printf("Failed to parse glTF\n");
      return;
    }

    initializeModelVertexData(&tinyGLTFModel, returnModels[i]);
  }
}

void drawModel(const Model& model) {
  for(u32 i = 0; i < model.meshCount; ++i) {
    Mesh* meshPtr = model.meshes + i;
    drawTriangles(&meshPtr->vertexAtt);
  }
}

void deleteModels(Model* models, u32 count) {
  std::vector<VertexAtt> vertexAtts;
  std::vector<GLuint> textureData;

  for(u32 i = 0; i < count; ++i) {
    Model* modelPtr = models + i;
    for(u32 i = 0; i < modelPtr->meshCount; ++i) {
      Mesh* meshPtr = modelPtr->meshes;
      vertexAtts.push_back(meshPtr->vertexAtt);
      TextureData textureDatum = meshPtr->textureData;
      if(textureDatum.normalTextureId != TEXTURE_ID_NO_TEXTURE) {
        textureData.push_back(textureDatum.normalTextureId);
      }
      if(textureDatum.albedoTextureId != TEXTURE_ID_NO_TEXTURE) {
        textureData.push_back(textureDatum.albedoTextureId);
      }
    }
    delete[] modelPtr->meshes;
    *modelPtr = {}; // clear model to zero
  }

  deleteVertexAtts(vertexAtts.data(), (u32)vertexAtts.size());
  glDeleteTextures((GLsizei)textureData.size(), textureData.data());
}