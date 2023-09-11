#pragma once

enum FramebufferCreationFlags {
  FramebufferCreate_NoValue = 0,
  FramebufferCreate_NoDepthStencil = 1 << 0,
  FramebufferCreate_color_sRGB = 1 << 1,
};

internal_func inline void bindActiveTexture(s32 activeIndex, GLuint textureId, GLenum target) {
  glActiveTexture(GL_TEXTURE0 + activeIndex);
  glBindTexture(target, textureId);
}

void inline bindActiveTextureSampler2d(s32 activeIndex, GLuint textureId) {
  bindActiveTexture(activeIndex, textureId, GL_TEXTURE_2D);
}

void inline bindActiveTextureCubeMap(s32 activeIndex, GLuint textureId) {
  bindActiveTexture(activeIndex, textureId, GL_TEXTURE_CUBE_MAP);
}

void load2DTexture(const char* imgLocation, u32* textureId, bool flipImageVert = false, bool inputSRGB = false, u32* width = NULL, u32* height = NULL)
{
  TimeFunction

  glGenTextures(1, textureId);
  glBindTexture(GL_TEXTURE_2D, *textureId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); // disables bilinear filtering (creates sharp edges when magnifying texture)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

  // load image data
  s32 w, h, numChannels;
  stbi_set_flip_vertically_on_load(flipImageVert);
  Asset imageAsset = Asset(assetManager_GLOBAL, imgLocation);
  if(!imageAsset.success()) {
    LOGI("Failed to find texture asset: %s", imgLocation);
    return;
  }
  u8* data = stbi_load_from_memory((stbi_uc const *)imageAsset.buffer, imageAsset.bufferLengthInBytes, &w, &h, &numChannels, 0 /*desired channels*/);
  if (data && numChannels <= 4)
  {
    u32 dataColorSpace;
    u32 dataComponentComposition;
    switch(numChannels) {
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
                 w, // width of texture
                 h, // height of texture
                 0, // border (legacy stuff, MUST BE 0)
                 dataComponentComposition, // How are the components of the data composed
                 GL_UNSIGNED_BYTE, // specifies data type of pixel data
                 data); // pointer to the image data
    glGenerateMipmap(GL_TEXTURE_2D);

    if (width != NULL) *width = w;
    if (height != NULL) *height = h;
  } else {
    LOGE("Texture asset found but failed to load image - %s", imgLocation);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data); // free texture image memory
}

void loadCubeMapTexture(const char* fileName, GLuint* textureId) {
  TimeFunction

  glGenTextures(1, textureId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, *textureId);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // TODO: This is NOT where exported assets directory should be stored. Move this or related solution to assetlib or potentially a asset_baker header.
  std::string bakedSkyboxesDir = "assets_export/skyboxes/";
  std::string assetPath = bakedSkyboxesDir + fileName + ".cbtx";

  assets::AssetFile cubeMapAssetFile;
  {
    TimeBlock("assets::loadAssetFile")
    assets::loadAssetFile(assetManager_GLOBAL, assetPath.c_str(), &cubeMapAssetFile);
  }
  assets::CubeMapInfo cubeMapInfo;
  {
    TimeBlock("assets::readCubeMapInfo")
    assets::readCubeMapInfo(cubeMapAssetFile, &cubeMapInfo);
  }
  char* cubeMapData;
  {
    TimeBlock("loadCubeMapTexture - malloc cubemap texture")
    cubeMapData = (char*)malloc(cubeMapInfo.size());
  }
  {
    TimeBlock("assets::unpackCubeMap")
    assets::unpackCubeMap(cubeMapInfo, cubeMapAssetFile.binaryBlob.data(), cubeMapAssetFile.binaryBlob.size(), cubeMapData);
  }

  {
    TimeBlock("loadCubeMapTexture - glTexImage2D")
    // TODO: If we ever support other formats besides RGB8 we will need to explicitly translate the CubeMapInfo.format to a GL_{format}
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_BACK));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_FRONT));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_BOTTOM));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_TOP));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_LEFT));
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_RIGHT));
  }

  {
    TimeBlock("loadCubeMapTexture - free cubemap texture")
    free(cubeMapData);
  }
}