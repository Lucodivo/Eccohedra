#include "cubemap_asset.h"

const internal_func char* CUBE_MAP_FOURCC = "CBMP";

const struct {
  const char* faceSize = "face_size";
  const char* format = "format";
  const char* formatEnum = "format_enum";
  const char* originalFolder = "original_folder";
  const char* faceWidth = "face_width";
  const char* faceHeight = "face_height";
} jsonKeys;

void assets::readCubeMapInfo(const assets::AssetFile &file, assets::CubeMapInfo *info) {
  nlohmann::json cubeMapJson = nlohmann::json::parse(file.json);
  u32 cubeMapFormatEnum = cubeMapJson[jsonKeys.formatEnum];
  info->format = TextureFormat(cubeMapFormatEnum);
  info->faceSize = cubeMapJson[jsonKeys.faceSize];
  info->faceWidth = cubeMapJson[jsonKeys.faceWidth];
  info->faceHeight = cubeMapJson[jsonKeys.faceHeight];
  info->originalFolder = cubeMapJson[jsonKeys.originalFolder];
}

assets::AssetFile assets::packCubeMap(CubeMapInfo *info, void *data_FBTBLR) {

  //core file header
  AssetFile file;
  strncpy(file.type, CUBE_MAP_FOURCC, 4);
  file.version = ASSET_LIB_VERSION;

  nlohmann::json cubeMapJson;
  cubeMapJson[jsonKeys.format] = textureFormatToString(TextureFormat_RGB8);
  cubeMapJson[jsonKeys.formatEnum] = textureFormatToEnumVal(TextureFormat_RGB8);
  cubeMapJson[jsonKeys.faceSize] = info->faceSize;
  cubeMapJson[jsonKeys.originalFolder] = info->originalFolder;
  cubeMapJson[jsonKeys.faceWidth] = info->faceWidth;
  cubeMapJson[jsonKeys.faceHeight] = info->faceHeight;
  file.json = cubeMapJson.dump(); // json map to string

  file.binaryBlob.resize(info->size());
  memcpy(&file.binaryBlob[0], data_FBTBLR, info->size());

  return file;
}