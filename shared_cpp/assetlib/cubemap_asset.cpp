#include "cubemap_asset.h"

const internal_func char* mapCubeMapFormatToString[] = {
    "Unknown",
#define CubeMapFormat(name, _) #name,
#include "cubemap_format.incl"
#undef CubeMapFormat
};

const internal_func char* CUBE_MAP_FOURCC = "CBMP";

const struct {
  const char* faceSize = "face_size";
  const char* compressionMode = "compression_mode";
  const char* compressionModeEnum = "compression_mode_enum";
  const char* format = "format";
  const char* formatEnum = "format_enum";
  const char* originalFolder = "original_folder";
  const char* faceWidth = "face_width";
  const char* faceHeight = "face_height";
  const char* compressedSize = "compressed_size";
} jsonKeys;

inline u32 cubeMapFormatToEnumVal(assets::CubeMapFormat format) { return static_cast<u32>(format); }
const char* cubeMapFormatToString(assets::CubeMapFormat format) { return mapCubeMapFormatToString[cubeMapFormatToEnumVal(format)]; }

void assets::readCubeMapInfo(const assets::AssetFile &file, assets::CubeMapInfo *info) {
  nlohmann::json cubeMapJson = nlohmann::json::parse(file.json);

  const std::string& formatString = cubeMapJson[jsonKeys.format];
  u32 cubeMapFormatEnum = cubeMapJson[jsonKeys.formatEnum];
  info->format = CubeMapFormat(cubeMapFormatEnum);

  const std::string& compressionString = cubeMapJson[jsonKeys.compressionMode];
  u32 compressionModeEnumVal = cubeMapJson[jsonKeys.compressionModeEnum];
  info->compressionMode = CompressionMode(compressionModeEnumVal);

  info->faceSize = cubeMapJson[jsonKeys.faceSize];
  info->originalFolder = cubeMapJson[jsonKeys.originalFolder];
  info->faceWidth = cubeMapJson[jsonKeys.faceWidth];
  info->faceHeight = cubeMapJson[jsonKeys.faceHeight];
}

void assets::unpackCubeMap(const assets::CubeMapInfo& info, char *srcBuff, size_t srcBuffSize, char *dest) {
  switch(info.compressionMode) {
    case CompressionMode::None:
      // TODO: Memcpy is potentially costly in this scenario. Change API to allow simply returning srcBuff.
      memcpy(dest, srcBuff, srcBuffSize);
      break;
    case CompressionMode::LZ4:
      LZ4_decompress_safe(srcBuff, dest, (s32)srcBuffSize, (s32)info.size());
      break;
  }
}

assets::AssetFile assets::packCubeMap(CubeMapInfo *info,
                    void *frontFaceData, void *backFaceData,
                    void *topFaceData, void *botFaceData,
                    void *leftFaceData, void *rightFaceData) {
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

  char* pixels = (char*)malloc(info->size());
  memcpy(info->faceData(pixels, SKYBOX_FACE_FRONT), frontFaceData, info->faceSize);
  memcpy(info->faceData(pixels, SKYBOX_FACE_BACK), backFaceData, info->faceSize);
  memcpy(info->faceData(pixels, SKYBOX_FACE_TOP), topFaceData, info->faceSize);
  memcpy(info->faceData(pixels, SKYBOX_FACE_BOTTOM), botFaceData, info->faceSize);
  memcpy(info->faceData(pixels, SKYBOX_FACE_LEFT), leftFaceData, info->faceSize);
  memcpy(info->faceData(pixels, SKYBOX_FACE_RIGHT), rightFaceData, info->faceSize);

  //compress buffer into blob
  std::vector<char> decompressionBuffer;
  s32 worstCaseCompressionSize = LZ4_compressBound((s32)(info->size()));
  decompressionBuffer.resize(worstCaseCompressionSize);
  u64 actualCompressionSize = LZ4_compress_default(pixels, decompressionBuffer.data(), (s32)info->size(), worstCaseCompressionSize);

  //if the compression is more than 80% of the original size, it's not worth to use it
  f64 compressionRate = f64(actualCompressionSize) / f64(info->size());
  if (compressionRate > 0.8f)
  {
    actualCompressionSize = info->size();
    decompressionBuffer.resize(actualCompressionSize);
    memcpy(decompressionBuffer.data(), pixels, actualCompressionSize);
    cubeMapJson[jsonKeys.compressionMode] = compressionModeToString(CompressionMode::None);
    cubeMapJson[jsonKeys.compressionModeEnum] = compressionModeToEnumVal(CompressionMode::None);
  } else {
    cubeMapJson[jsonKeys.compressionMode] = compressionModeToString(CompressionMode::LZ4);
    cubeMapJson[jsonKeys.compressionModeEnum] = compressionModeToEnumVal(CompressionMode::LZ4);
  }

  cubeMapJson[jsonKeys.compressedSize] = actualCompressionSize;

  decompressionBuffer.resize(actualCompressionSize);
  file.binaryBlob.insert(file.binaryBlob.end(), decompressionBuffer.begin(), decompressionBuffer.end());

  // json map to string
  std::string cubeMapMetadataJsonString = cubeMapJson.dump();
  file.json = cubeMapMetadataJsonString;

  free(pixels);

  return file;
}