#include "texture_asset.h"

const internal_func char* mapTextureFormatToString[] = {
        "Unknown",
#define TextureFormat(name) #name,
#include "texture_format.incl"
#undef TextureFormat
};

const internal_func char* TEXTURE_FOURCC = "TEXI";

const struct {
  const char* textureSize = "texture_size";
  const char* compressionMode = "compression_mode";
  const char* compressionModeEnumVal = "compression_mode_enum_val";
  const char* textureFormat = "texture_format";
  const char* textureFormatEnumVal = "texture_format_enum_val";
  const char* originalFile = "original_file";
  const char* width = "width";
  const char* height = "height";
  const char* compressedSize = "compressed_size";
} jsonKeys;

const char* textureFormatToString(assets::TextureFormat format);
u32 textureFormatToEnumVal(assets::TextureFormat format);

void assets::readTextureInfo(const AssetFile& file, TextureInfo* texInfo) {
  nlohmann::json textureJson = nlohmann::json::parse(file.json);

  const std::string& formatString = textureJson[jsonKeys.textureFormat];
  u32 textureFormatEnumVal = textureJson[jsonKeys.textureFormatEnumVal];
  texInfo->textureFormat = TextureFormat(textureFormatEnumVal);

  const std::string& compressionString = textureJson[jsonKeys.compressionMode];
  u32 compressionModeEnumVal = textureJson[jsonKeys.compressionModeEnumVal];
  texInfo->compressionMode = CompressionMode(compressionModeEnumVal);

  texInfo->textureSize = textureJson[jsonKeys.textureSize];
  texInfo->originalFile = textureJson[jsonKeys.originalFile];
  texInfo->width = textureJson[jsonKeys.width];
  texInfo->height = textureJson[jsonKeys.height];
}

void assets::unpackTexture(const TextureInfo& texInfo, const char* sourceBuffer, size_t sourceSize, char* destination) {
  switch(texInfo.compressionMode) {
    case CompressionMode::None:
      memcpy(destination, sourceBuffer, sourceSize);
      break;
    case CompressionMode::LZ4:
      LZ4_decompress_safe(sourceBuffer, destination, (s32)sourceSize, (s32)texInfo.textureSize);
      break;
  }
}

assets::AssetFile assets::packTexture(TextureInfo* info, void* pixelData) {

  //core file header
  AssetFile file;
  strncpy(file.type, TEXTURE_FOURCC, 4);
  file.version = ASSET_LIB_VERSION;

  char* pixels = (char*)pixelData;
  std::vector<char> decompressionBuffer;

  //compress buffer into blob
  s32 worstCaseCompressionSize = LZ4_compressBound((s32)info->textureSize);
  decompressionBuffer.resize(worstCaseCompressionSize);
  u64 actualCompressionSize = LZ4_compress_default(pixels, decompressionBuffer.data(), (s32)info->textureSize, worstCaseCompressionSize);

  f64 compressionRate = f64(actualCompressionSize) / f64(info->textureSize);

  //if the compression is more than 80% of the original size, it's not worth to use it
  if (compressionRate > 0.8f)
  {
    actualCompressionSize = info->textureSize;
    decompressionBuffer.resize(actualCompressionSize);
    memcpy(decompressionBuffer.data(), pixels, actualCompressionSize);
  } else {
    decompressionBuffer.resize(actualCompressionSize);
  }

  file.binaryBlob.insert(file.binaryBlob.end(), decompressionBuffer.begin(), decompressionBuffer.end());

  nlohmann::json textureJson;
  textureJson[jsonKeys.textureFormat] = textureFormatToString(TextureFormat::RGBA8);
  textureJson[jsonKeys.textureFormatEnumVal] = textureFormatToEnumVal(TextureFormat::RGBA8);
  textureJson[jsonKeys.textureSize] = info->textureSize;
  textureJson[jsonKeys.originalFile] = info->originalFile;
  textureJson[jsonKeys.compressionMode] = compressionModeToString(CompressionMode::LZ4);
  textureJson[jsonKeys.compressionModeEnumVal] = compressionModeToEnumVal(CompressionMode::LZ4);
  textureJson[jsonKeys.width] = info->width;
  textureJson[jsonKeys.height] = info->height;
  textureJson[jsonKeys.compressedSize] = actualCompressionSize;

  // json map to string
  std::string texMetadataJsonString = textureJson.dump();
  file.json = texMetadataJsonString;

  return file;
}

const char* textureFormatToString(assets::TextureFormat format) {
  return mapTextureFormatToString[textureFormatToEnumVal(format)];
}

inline u32 textureFormatToEnumVal(assets::TextureFormat format) {
  return static_cast<u32>(format);
}