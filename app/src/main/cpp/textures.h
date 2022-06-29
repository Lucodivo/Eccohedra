#pragma once

#define NO_FRAMEBUFFER_ATTACHMENT 0

struct Framebuffer {
  u32 id;
  u32 colorAttachment;
  u32 depthStencilAttachment;
  vec2_u32 extent;
};

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
  u8* data = stbi_load(imgLocation, &w, &h, &numChannels, 0 /*desired channels*/);
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
  } else
  {
    LOGE("Failed to load texture");
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(data); // free texture image memory
}

void loadCubeMapTexture(const char* directory, const char* extension, GLuint* textureId, bool flipImageVert = false) {

  const char* skyboxTextureTitles[] = {
          "front.",
          "back.",
          "top.",
          "bottom.",
          "right.",
          "left.",
  };
  const u32 maxTextureFileLength = 128;
  size_t directoryLength = std::strlen(directory);
  char skyboxTextureFileNameBuffer[maxTextureFileLength];
  std::strcpy(skyboxTextureFileNameBuffer, directory);

  glGenTextures(1, textureId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, *textureId);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  s32 width, height, nrChannels;
  stbi_set_flip_vertically_on_load(flipImageVert);

  auto loadCubeMapFace = [&width, &height, &nrChannels](GLenum faceTarget, const char* faceImageLoc){
    unsigned char* data = stbi_load(faceImageLoc, &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(faceTarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else
    {
      LOGE("Cubemap texture failed to load at path: %s", faceImageLoc);
    }
    stbi_image_free(data);
  };

  size_t titleLength = std::strlen(skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_BACK]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength, skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_BACK]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength + titleLength, extension);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, skyboxTextureFileNameBuffer);

  titleLength = std::strlen(skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_FRONT]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength, skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_FRONT]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength + titleLength, extension);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, skyboxTextureFileNameBuffer);


  titleLength = std::strlen(skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_BOTTOM]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength, skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_BOTTOM]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength + titleLength, extension);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, skyboxTextureFileNameBuffer);


  titleLength = std::strlen(skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_TOP]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength, skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_TOP]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength + titleLength, extension);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, skyboxTextureFileNameBuffer);


  titleLength = std::strlen(skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_LEFT]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength, skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_LEFT]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength + titleLength, extension);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, skyboxTextureFileNameBuffer);


  titleLength = std::strlen(skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_RIGHT]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength, skyboxTextureTitles[SKYBOX_TEXTURE_LOCATION_INDEX_RIGHT]);
  strcpy(skyboxTextureFileNameBuffer + directoryLength + titleLength, extension);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, skyboxTextureFileNameBuffer);
}

void loadCubeMapTexture(const char* const imgLocations[6], GLuint* textureId, bool flipImageVert = false)
{
  glGenTextures(1, textureId);
  glBindTexture(GL_TEXTURE_CUBE_MAP, *textureId);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  s32 width, height, nrChannels;
  stbi_set_flip_vertically_on_load(flipImageVert);

  auto loadCubeMapFace = [&width, &height, &nrChannels](GLenum faceTarget, const char* faceImageLoc){
    unsigned char* data = stbi_load(faceImageLoc, &width, &height, &nrChannels, 0);
    if (data)
    {
      glTexImage2D(faceTarget, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else
    {
      LOGE("Cubemap texture failed to load at path: %s", faceImageLoc);
    }
    stbi_image_free(data);
  };

  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, imgLocations[SKYBOX_TEXTURE_LOCATION_INDEX_BACK]);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_X, imgLocations[SKYBOX_TEXTURE_LOCATION_INDEX_FRONT]);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, imgLocations[SKYBOX_TEXTURE_LOCATION_INDEX_BOTTOM]);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, imgLocations[SKYBOX_TEXTURE_LOCATION_INDEX_TOP]);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, imgLocations[SKYBOX_TEXTURE_LOCATION_INDEX_LEFT]);
  loadCubeMapFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, imgLocations[SKYBOX_TEXTURE_LOCATION_INDEX_RIGHT]);
}

Framebuffer initializeFramebuffer(vec2_u32 framebufferExtent, FramebufferCreationFlags flags = FramebufferCreate_NoValue)
{
  Framebuffer resultBuffer;
  resultBuffer.extent = framebufferExtent;

  GLint originalDrawFramebuffer, originalReadFramebuffer, originalActiveTexture, originalTexture0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalDrawFramebuffer);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &originalReadFramebuffer);
  glGetIntegerv(GL_ACTIVE_TEXTURE, &originalActiveTexture);

  // creating frame buffer
  glGenFramebuffers(1, &resultBuffer.id);
  glBindFramebuffer(GL_FRAMEBUFFER, resultBuffer.id);

  // creating frame buffer color texture
  glGenTextures(1, &resultBuffer.colorAttachment);
  // NOTE: Binding the texture to the GL_TEXTURE_2D target, means that
  // NOTE: gl operations on the GL_TEXTURE_2D target will affect our texture
  // NOTE: while it is remains bound to that target
  glActiveTexture(GL_TEXTURE0);
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &originalTexture0);
  glBindTexture(GL_TEXTURE_2D, resultBuffer.colorAttachment);
  GLint internalFormat = (flags & FramebufferCreate_color_sRGB) ? GL_SRGB : GL_RGB;
  glTexImage2D(GL_TEXTURE_2D, 0/*LoD*/, internalFormat, framebufferExtent.width, framebufferExtent.height, 0/*border*/, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // attach texture w/ color to frame buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, // frame buffer we're targeting (draw, read, or both)
                         GL_COLOR_ATTACHMENT0, // type of attachment and index of attachment
                         GL_TEXTURE_2D, // type of texture
                         resultBuffer.colorAttachment, // texture
                         0); // mipmap level

  if (flags & FramebufferCreate_NoDepthStencil)
  {
    resultBuffer.depthStencilAttachment = NO_FRAMEBUFFER_ATTACHMENT;
  } else {
    // creating render buffer to be depth/stencil buffer
    glGenRenderbuffers(1, &resultBuffer.depthStencilAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, resultBuffer.depthStencilAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferExtent.width, framebufferExtent.height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0); // unbind
    // attach render buffer w/ depth & stencil to frame buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, // frame buffer target
                              GL_DEPTH_STENCIL_ATTACHMENT, // attachment po of frame buffer
                              GL_RENDERBUFFER, // render buffer target
                              resultBuffer.depthStencilAttachment);  // render buffer
  }

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    LOGE("ERROR::FRAMEBUFFER:: Framebuffer is not complete!");
  }
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalDrawFramebuffer);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, originalReadFramebuffer);
  glBindTexture(GL_TEXTURE_2D, originalTexture0); // re-bind original texture
  glActiveTexture(originalActiveTexture);
  return resultBuffer;
}

void deleteFramebuffer(Framebuffer* framebuffer)
{
  glDeleteFramebuffers(1, &framebuffer->id);
  glDeleteTextures(1, &framebuffer->colorAttachment);
  if (framebuffer->depthStencilAttachment != NO_FRAMEBUFFER_ATTACHMENT)
  {
    glDeleteRenderbuffers(1, &framebuffer->depthStencilAttachment);
  }
  *framebuffer = {0, 0, 0, 0, 0};
}

void deleteFramebuffers(u32 count, Framebuffer** framebuffer)
{
  u32* deleteFramebufferObjects = new u32[count * 3];
  u32* deleteColorAttachments = deleteFramebufferObjects + count;
  u32* deleteDepthStencilAttachments = deleteColorAttachments + count;
  u32 depthStencilCount = 0;
  for (u32 i = 0; i < count; i++)
  {
    deleteFramebufferObjects[i] = framebuffer[i]->id;
    deleteColorAttachments[i] = framebuffer[i]->colorAttachment;
    if (framebuffer[i]->depthStencilAttachment != NO_FRAMEBUFFER_ATTACHMENT)
    {
      deleteDepthStencilAttachments[depthStencilCount++] = framebuffer[i]->depthStencilAttachment;
      *(framebuffer[i]) = {0, 0, 0, 0, 0};
    }
  }

  glDeleteFramebuffers(count, deleteFramebufferObjects);
  glDeleteTextures(count, deleteColorAttachments);
  if (depthStencilCount != 0)
  {
    glDeleteRenderbuffers(depthStencilCount, deleteDepthStencilAttachments);
  }

  delete[] deleteFramebufferObjects;
}