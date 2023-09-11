#include "asset_loader.h"

const internal_func char* mapCompressionModeToString[] = {
        "None",
#define CompressionMode(name) #name,
#include "compression_mode.incl"
#undef CompressionMode
};

#if defined(ANDROID) || defined(__ANDROID___)
bool assets::loadAssetFile(AAssetManager* assetManager, const char* path, AssetFile* outputFile) {
  AAsset *androidAsset = AAssetManager_open(assetManager, path, AASSET_MODE_STREAMING);

  if(androidAsset == nullptr) {
    LOGI("Asset manager could not find asset: ", path);
    return false;
  }

  // file type
  AAsset_read(androidAsset, outputFile->type, FILE_TYPE_SIZE_IN_BYTES);

  // version
  AAsset_read(androidAsset, &outputFile->version, sizeof(int));
  if(outputFile->version != ASSET_LIB_VERSION) {
    LOGI("Attempting to load asset (%s) with version #%d. Asset Loader version is currently #%d.", path, outputFile->version, ASSET_LIB_VERSION);
    return false;
  }

  // json length
  size_t jsonLength;
  AAsset_read(androidAsset, &jsonLength, sizeof(size_t));

  // blob length
  size_t blobLength;
  AAsset_read(androidAsset, &blobLength, sizeof(size_t));

  // json
  outputFile->json.resize(jsonLength);
  AAsset_read(androidAsset, outputFile->json.data(), (ptrdiff_t)jsonLength);

  // blob
  outputFile->binaryBlob.resize(blobLength);
  AAsset_read(androidAsset, outputFile->binaryBlob.data(), (ptrdiff_t)blobLength);

  AAsset_close(androidAsset);

  return true;
}
#else

#include <fstream>

bool assets::saveAssetFile(const char* path, const AssetFile& file) {
  std::ofstream outfile;
  outfile.open(path, std::ios::binary | std::ios::out);

  if(!outfile.is_open()) {
    printf("Failed to export asset file: %s\n", path);
    return false;
  }

  // file type
  outfile.write(file.type, FILE_TYPE_SIZE_IN_BYTES);

  // version
  outfile.write((const char*)&file.version, sizeof(int));

  // json length
  size_t jsonLength = file.json.size();
  outfile.write((const char*)&jsonLength, sizeof(size_t));

  // blob length
  size_t blobLength = file.binaryBlob.size();
  outfile.write((const char*)&blobLength, sizeof(size_t));

  //json
  outfile.write(file.json.data(), (ptrdiff_t)jsonLength);

  //blob data
  outfile.write(file.binaryBlob.data(), (ptrdiff_t)blobLength);

  outfile.close();

  return true;
}

bool assets::loadAssetFile(const char* path, AssetFile* outputFile) {
  std::ifstream infile;
  infile.open(path, std::ios::binary);

  if (!infile.is_open()) {
    printf("Could not open asset file %s", path);
    return false;
  }

  //move file cursor to beginning
  infile.seekg(0);

  // file type
  infile.read(outputFile->type, FILE_TYPE_SIZE_IN_BYTES);

  // version
  infile.read((char*)&outputFile->version, sizeof(u32));
  if(outputFile->version != ASSET_LIB_VERSION) {
    printf("Attempting to load asset (%s) with version #%d. Asset Loader version is currently #%d.", path, outputFile->version, ASSET_LIB_VERSION);
  }

  // json length
  size_t jsonLength;
  infile.read((char*)&jsonLength, sizeof(size_t));

  // blob length
  size_t blobLength;
  infile.read((char*)&blobLength, sizeof(size_t));

  // json
  outputFile->json.resize(jsonLength);
  infile.read(outputFile->json.data(), (ptrdiff_t)jsonLength);

  // blob
  outputFile->binaryBlob.resize(blobLength);
  infile.read(outputFile->binaryBlob.data(), (ptrdiff_t)blobLength);

  return true;
}
#endif

const char* assets::compressionModeToString(CompressionMode compressionMode) {
  return mapCompressionModeToString[compressionModeToEnumVal(compressionMode)];
}

u32 assets::compressionModeToEnumVal(CompressionMode compressionMode) {
  return static_cast<u32>(compressionMode);
}