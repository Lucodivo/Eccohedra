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

  // TODO: This is NOT where exported assets directory should be stored. Move this or related solution to assetlib or potentially a asset_baker header.
  std::string textureDir = "textures/";
  std::string assetPath = textureDir + imgLocation + ".tx";

  // TODO: Investigate what can be done, if anything, to load cubemap assets faster
  assets::AssetFile textureAssetFile;
  {
    TimeBlock("load2DTexture - assets::loadAssetFile")
    assets::loadAssetFile(assetManager_GLOBAL, assetPath.c_str(), &textureAssetFile);
  }

  assets::TextureInfo textureInfo;
  {
    assets::readTextureInfo(textureAssetFile, &textureInfo);
  }

  char* textureData = textureAssetFile.binaryBlob.data();

  if(textureInfo.format == assets::TextureFormat_R8) {
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R8,
                 textureInfo.width,
                 textureInfo.height,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 textureData);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else if (textureInfo.format == assets::TextureFormat_ETC2_RGB) {
    glCompressedTexImage2D(GL_TEXTURE_2D,
                           0,
                           GL_COMPRESSED_RGB8_ETC2,
                           textureInfo.width,
                           textureInfo.height,
                           0,
                           textureInfo.size,
                           textureData);
    // TODO: Can you generate compressed mipmaps?
  } else {
    InvalidCodePath
  }

  if (width != NULL) *width = textureInfo.width;
  if (height != NULL) *height = textureInfo.height;

  glBindTexture(GL_TEXTURE_2D, 0);
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
  std::string bakedSkyboxesDir = "skyboxes/";
  std::string assetPath = bakedSkyboxesDir + fileName + ".cbtx";

  // TODO: Investigate what can be done, if anything, to load cubemap assets faster
  assets::AssetFile cubeMapAssetFile;
  {
    TimeBlock("loadCubeMapTexture - assets::loadAssetFile")
    assets::loadAssetFile(assetManager_GLOBAL, assetPath.c_str(), &cubeMapAssetFile);
  }

  assets::CubeMapInfo cubeMapInfo;
  assets::readCubeMapInfo(cubeMapAssetFile, &cubeMapInfo);

  {
    TimeBlock("loadCubeMapTexture - glTexImage2D")
    char* cubeMapData = cubeMapAssetFile.binaryBlob.data();
    GLenum compressionFormat = GL_COMPRESSED_RGB8_ETC2;
    // TODO: If we ever support other formats besides RGB8 we will need to explicitly translate the CubeMapInfo.format to a GL_{format}
    glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, compressionFormat, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, cubeMapInfo.faceSize, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_FRONT));
    glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, compressionFormat, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, cubeMapInfo.faceSize, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_BACK));
    glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, compressionFormat, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, cubeMapInfo.faceSize, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_TOP));
    glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, compressionFormat, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, cubeMapInfo.faceSize, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_BOTTOM));
    glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, compressionFormat, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, cubeMapInfo.faceSize, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_RIGHT));
    glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, compressionFormat, cubeMapInfo.faceWidth, cubeMapInfo.faceHeight, 0, cubeMapInfo.faceSize, cubeMapInfo.faceData(cubeMapData, SKYBOX_FACE_LEFT));
  }

  // TODO: Can you generate compressed mipmaps?
}