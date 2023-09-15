#include "texture_asset.h"

const internal_func char* mapTextureFormatToString[] = {
    "Unknown",
#define Texture(name) #name,
#include "texture_format.incl"
#undef Texture
};

const internal_func char* TEXTURE_FOURCC = "TEXI";

const struct {
  const char* size = "size";
  const char* format = "format";
  const char* formatEnum = "format_enum";
  const char* originalFileName = "original_file_name";
  const char* width = "width";
  const char* height = "height";
} jsonKeys;

inline u32 textureFormatToEnumVal(assets::TextureFormat format) { return static_cast<u32>(format); }
const char* textureFormatToString(assets::TextureFormat format) { return mapTextureFormatToString[textureFormatToEnumVal(format)]; }

void assets::readTextureInfo(const assets::AssetFile &file, assets::TextureInfo *info) {
  nlohmann::json cubeMapJson = nlohmann::json::parse(file.json);

  const std::string& formatString = cubeMapJson[jsonKeys.format];
  u32 cubeMapFormatEnum = cubeMapJson[jsonKeys.formatEnum];
  info->format = TextureFormat(cubeMapFormatEnum);
  info->size = cubeMapJson[jsonKeys.size];
  info->width = cubeMapJson[jsonKeys.width];
  info->height = cubeMapJson[jsonKeys.height];
  info->originalFileName = cubeMapJson[jsonKeys.originalFileName];
}

assets::AssetFile assets::packTexture(TextureInfo* info, void *data) {

  //core file header
  AssetFile file;
  strncpy(file.type, TEXTURE_FOURCC, 4);
  file.version = ASSET_LIB_VERSION;

  nlohmann::json textureJson;
  textureJson[jsonKeys.format] = textureFormatToString(TextureFormat::RGB8);
  textureJson[jsonKeys.formatEnum] = textureFormatToEnumVal(TextureFormat::RGB8);
  textureJson[jsonKeys.size] = info->size;
  textureJson[jsonKeys.originalFileName] = info->originalFileName;
  textureJson[jsonKeys.width] = info->width;
  textureJson[jsonKeys.height] = info->height;
  file.json = textureJson.dump(); // json map to string

  file.binaryBlob.resize(info->size);
  memcpy(&file.binaryBlob[0], data, info->size);

  return file;
}