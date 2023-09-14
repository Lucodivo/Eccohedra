#include "cubemap_asset.h"

const internal_func char* mapCubeMapFormatToString[] = {
    "Unknown",
#define CubeMapFormat(name) #name,
#include "cubemap_format.incl"
#undef CubeMapFormat
};

const internal_func char* CUBE_MAP_FOURCC = "CBMP";

const struct {
  const char* faceSize = "face_size";
  const char* format = "format";
  const char* formatEnum = "format_enum";
  const char* originalFolder = "original_folder";
  const char* faceWidth = "face_width";
  const char* faceHeight = "face_height";
} jsonKeys;

inline u32 cubeMapFormatToEnumVal(assets::CubeMapFormat format) { return static_cast<u32>(format); }
const char* cubeMapFormatToString(assets::CubeMapFormat format) { return mapCubeMapFormatToString[cubeMapFormatToEnumVal(format)]; }

void assets::readCubeMapInfo(const assets::AssetFile &file, assets::CubeMapInfo *info) {
  nlohmann::json cubeMapJson = nlohmann::json::parse(file.json);

  const std::string& formatString = cubeMapJson[jsonKeys.format];
  u32 cubeMapFormatEnum = cubeMapJson[jsonKeys.formatEnum];
  info->format = CubeMapFormat(cubeMapFormatEnum);
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
  cubeMapJson[jsonKeys.format] = cubeMapFormatToString(CubeMapFormat::RGB8);
  cubeMapJson[jsonKeys.formatEnum] = cubeMapFormatToEnumVal(CubeMapFormat::RGB8);
  cubeMapJson[jsonKeys.faceSize] = info->faceSize;
  cubeMapJson[jsonKeys.originalFolder] = info->originalFolder;
  cubeMapJson[jsonKeys.faceWidth] = info->faceWidth;
  cubeMapJson[jsonKeys.faceHeight] = info->faceHeight;
  file.json = cubeMapJson.dump(); // json map to string

  file.binaryBlob.resize(info->size());
  memcpy(&file.binaryBlob[0], data_FBTBLR, info->size());

  return file;
}

void assets::unpackCubeMap(const CubeMapInfo& texInfo, char* srcBuff, size_t srcBuffSize, char* dest) {
  memcpy(dest, srcBuff, srcBuffSize);
}