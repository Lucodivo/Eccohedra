#include "asset_loader.h"

#include <fstream>

const internal_func char* mapCompressionModeToString[] = {
        "None",
#define CompressionMode(name) #name,
#include "compression_mode.incl"
#undef CompressionMode
};

#ifndef ANDROID
bool assets::saveAssetFile(const char* path, const AssetFile& file) {
  std::ofstream outfile;
  outfile.open(path, std::ios::binary | std::ios::out);

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
#else
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

inline u32 assets::compressionModeToEnumVal(CompressionMode compressionMode) {
  return static_cast<u32>(compressionMode);
}