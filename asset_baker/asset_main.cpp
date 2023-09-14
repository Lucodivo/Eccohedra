#include <iostream>
#include <unordered_set>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include "lz4/lz4.h"
#include <chrono>
#include "nlohmann/json.hpp"
#include "compressonator.h"

#include "stb/stb_image.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "tinygltf/tiny_gltf.h"

#include "noop_types.h"

#include "util.cpp"

#include "asset_loader.h"
#include "cubemap_asset.h"
using namespace assets;

#include "noop_math.h"
using namespace noop;

struct {
  const char* png = ".png";
  const char* jpg = ".jpg";
  const char* tga = ".TGA";
  const char* obj = ".obj";
  const char* gltf = ".gltf";
  const char* glb = ".glb";
} supportedFileExtensions;

struct {
  const char* texture = ".tx";
  const char* mesh = ".mesh";
  const char* material = ".mat";
  const char* prefab = ".pfb";
  const char* cubeMap = ".cbtx";
} bakedExtensions;

struct {
  const char* assetBakerCacheFileName = "Asset-Baker-Cache.asb";
  const char* cacheFiles = "cacheFiles";
  const char* originalFileName = "originalFileName";
  const char* originalFileLastModified = "originalFileLastModified";
  const char* bakedFiles = "bakedFiles";
  const char* fileName = "fileName";
  const char* fileExt = "fileExt";
  const char* filePath = "filePath";
} cacheJsonStrings;

struct ConverterState {
  fs::path assetsDir;
  fs::path bakedAssetDir;
  fs::path outputFileDir;
  std::vector<fs::path> bakedFilePaths;

  fs::path convertToExportRelative(const fs::path& path) const;
};

// TODO: Caching should note the version of the AssetLib used when asset was baked
struct AssetBakeCachedItem {
  struct BakedFile {
    std::string path;
    std::string ext;
    std::string name;
  };

  std::string originalFileName;
  f64 originalFileLastModified;
  std::vector<BakedFile> bakedFiles;
};

bool convertImage(const fs::path& inputPath, ConverterState& converterState);
bool convertCubeMapTexture(const fs::path& inputDir, ConverterState* converterState);

//void packVertex(assets::Vertex_PNCV_f32& new_vert, tinyobj::real_t vx, tinyobj::real_t vy, tinyobj::real_t vz, tinyobj::real_t nx, tinyobj::real_t ny, tinyobj::real_t nz, tinyobj::real_t ux, tinyobj::real_t uy);
//void packVertex(assets::Vertex_P32N8C8V16& new_vert, tinyobj::real_t vx, tinyobj::real_t vy, tinyobj::real_t vz, tinyobj::real_t nx, tinyobj::real_t ny, tinyobj::real_t nz, tinyobj::real_t ux, tinyobj::real_t uy);

//void unpackGltfBuffer(tinygltf::Model& model, tinygltf::Accessor& accessor, std::vector<u8>& outputBuffer);
//void extractGltfVertices(tinygltf::Primitive& primitive, tinygltf::Model& model, std::vector<assets::Vertex_PNCV_f32>& _vertices);
//void extractGltfIndices(tinygltf::Primitive& primitive, tinygltf::Model& model, std::vector<u32>& _primIndices);
//bool extractGltfMeshes(tinygltf::Model& gltfModel, const std::string& filePath, const fs::path& outputFolder, ConverterState& converterState);
//bool extractGltfCombinedMesh(tinygltf::Model& gltfModel, const fs::path& filePath, const fs::path& outputFolder, ConverterState& converterState); // combines all meshes into a single large mesh
//void extractGltfMaterials(tinygltf::Model& model, const fs::path& input, const fs::path& outputFolder, ConverterState& converterState);
//void extractGltfNodes(tinygltf::Model& model, const fs::path& input, const fs::path& outputFolder, ConverterState& converterState);
//std::string calculateGltfMaterialName(tinygltf::Model& model, int materialIndex);
//std::string calculateGltfMeshName(tinygltf::Model& model, int meshIndex, int primitiveIndex);

bool extractObjCombinedMesh(tinyobj::ObjReader& objReader, const fs::path& filePath, const fs::path& outputFolder, ConverterState& converterState);

void saveCache(const std::unordered_map<std::string, AssetBakeCachedItem>& oldCache, const std::vector<AssetBakeCachedItem>& newBakedItems);
void loadCache(std::unordered_map<std::string, AssetBakeCachedItem>& assetBakeCache);

void writeOutputData(const std::unordered_map<std::string, AssetBakeCachedItem>& oldCache, const ConverterState& converterState);
void replace(std::string& str, const char oldToken, const char newToken);
void replaceBackSlashes(std::string& str);
std::size_t fileCountInDir(fs::path dirPath);
std::size_t dirCountInDir(fs::path dirPath);

f64 lastModifiedTimeStamp(const fs::path& file) {
  auto lastModifiedTimePoint = fs::last_write_time(file);
  f64 lastModified = (f64)(lastModifiedTimePoint.time_since_epoch().count());
  return lastModified;
}

bool fileUpToDate(const std::unordered_map<std::string, AssetBakeCachedItem>& cache, fs::path file) {
  std::string fileName = file.filename().string();
  auto cachedItem = cache.find(fileName);
  if(cachedItem == cache.end()) {
    return false;
  }

  f64 lastModified = lastModifiedTimeStamp(file);

  bool upToDate = epsilonComparison(lastModified, cachedItem->second.originalFileLastModified);

  if(upToDate) {
    printf("Asset file \"%s\" is up-to-date\n", fileName.c_str());
  }

  return upToDate;
}

fs::path ConverterState::convertToExportRelative(const fs::path& path) const {
  return path.lexically_proximate(bakedAssetDir);
}

std::size_t fileCountInDir(fs::path dirPath) {
  std::size_t fileCount = 0u;
  for(auto const& file: std::filesystem::directory_iterator(dirPath)) {
    if(fs::is_regular_file(file)) {
      ++fileCount;
    }
  }
  return fileCount;
}

std::size_t dirCountInDir(fs::path dirPath) {
  std::size_t dirCount = 0u;
  for(auto const& file: std::filesystem::directory_iterator(dirPath)) {
    if(fs::is_directory(file)) {
      ++dirCount;
    }
  }
  return dirCount;
}

void replace(std::string& str, const char* oldTokens, u32 oldTokensCount, char newToken) {
  for(char& c: str) {
    for(u32 i = 0; i < oldTokensCount; i++) {
      if(c == oldTokens[i]) {
        c = newToken;
        break;
      }
    }
  }
}

void replaceBackSlashes(std::string& str) {
  for(char& c: str) {
    if(c == '\\') {
      c = '/';
    }
  }
}

void writeOutputData(const std::unordered_map<std::string, AssetBakeCachedItem>& oldCache, const ConverterState& converterState) {
  if(!fs::is_directory(converterState.outputFileDir)) {
    fs::create_directory(converterState.outputFileDir);
  }

  std::ofstream outTexturesFile, outMeshFile, outMaterialFile, outPrefabFile;
  outTexturesFile.open((converterState.outputFileDir / "baked_textures.incl").string(), std::ios::out);
  outMeshFile.open((converterState.outputFileDir / "baked_meshes.incl").string(), std::ios::out);
  outMaterialFile.open((converterState.outputFileDir / "baked_materials.incl").string(), std::ios::out);
  outPrefabFile.open((converterState.outputFileDir / "baked_prefabs.incl").string(), std::ios::out);

  auto write = [&](std::string bakedPath, std::string originalFileName, const char* fileExt) {
    const char tokensToReplace[] = {'.', '-'};
    replace(originalFileName, tokensToReplace, ArrayCount(tokensToReplace), '_');
    replaceBackSlashes(bakedPath);
    if(strcmp(fileExt, bakedExtensions.texture) == 0) {
      outTexturesFile << "BakedTexture(" << originalFileName << ",\"" << bakedPath << "\")\n";
    } else if(strcmp(fileExt, bakedExtensions.material) == 0) {
      outMaterialFile << "BakedMaterial(" << originalFileName << ",\"" << bakedPath << "\")\n";
    } else if(strcmp(fileExt, bakedExtensions.mesh) == 0) {
      outMeshFile << "BakedMesh(" << originalFileName << ",\"" << bakedPath << "\")\n";
    } else if(strcmp(fileExt, bakedExtensions.prefab) == 0) {
      outPrefabFile << "BakedPrefab(" << originalFileName << ",\"" << bakedPath << "\")\n";
    }
  };

  for(const fs::path& path: converterState.bakedFilePaths) {
    std::string extensionStr = path.extension().string();
    std::string fileName = path.filename().replace_extension("").string();
    std::string filePath = path.string();
    write(filePath, fileName, extensionStr.c_str());
  }

  for(auto [originalFileName, cachedItem] : oldCache) {
    u32 cachedBakedFileCount = (u32)cachedItem.bakedFiles.size();
    for(u32 i = 0; i < cachedBakedFileCount; i++) {
      const AssetBakeCachedItem::BakedFile& bakedFile = cachedItem.bakedFiles[i];
      std::string extensionStr = bakedFile.ext;
      std::string fileName = std::string(bakedFile.name.begin(), bakedFile.name.end() - bakedFile.ext.size());
      std::string filePath = bakedFile.path;
      write(filePath, fileName, extensionStr.c_str());
    }
  }

  outTexturesFile.close();
  outMeshFile.close();
  outMaterialFile.close();
  outPrefabFile.close();
}

void saveCache(const std::unordered_map<std::string, AssetBakeCachedItem>& oldCache, const std::vector<AssetBakeCachedItem>& newBakedItems) {
  nlohmann::json cacheJson;

  nlohmann::json bakedFiles;
  for(auto& [fileName, oldCacheItem] : oldCache) {
    nlohmann::json newCacheItemJson;
    newCacheItemJson[cacheJsonStrings.originalFileName] = oldCacheItem.originalFileName;
    newCacheItemJson[cacheJsonStrings.originalFileLastModified] = oldCacheItem.originalFileLastModified;
    nlohmann::json newCacheBakedFiles;
    u32 bakedFileCount = (u32)oldCacheItem.bakedFiles.size();
    for(u32 i = 0; i < bakedFileCount; i++) {
      nlohmann::json newCacheBakedFile;
      const AssetBakeCachedItem::BakedFile& bakedFile = oldCacheItem.bakedFiles[i];
      newCacheBakedFile[cacheJsonStrings.filePath] = bakedFile.path;
      newCacheBakedFile[cacheJsonStrings.fileName] = bakedFile.name;
      newCacheBakedFile[cacheJsonStrings.fileExt] = bakedFile.ext;
      newCacheBakedFiles.push_back(newCacheBakedFile);
    }
    newCacheItemJson[cacheJsonStrings.bakedFiles] = newCacheBakedFiles;
    bakedFiles.push_back(newCacheItemJson);
  }
  for(auto& newCacheItem : newBakedItems) {
    nlohmann::json newCacheItemJson;
    newCacheItemJson[cacheJsonStrings.originalFileName] = newCacheItem.originalFileName;
    newCacheItemJson[cacheJsonStrings.originalFileLastModified] = newCacheItem.originalFileLastModified;
    nlohmann::json newCacheBakedFiles;
    u32 bakedFileCount = (u32)newCacheItem.bakedFiles.size();
    for(u32 i = 0; i < bakedFileCount; i++) {
      nlohmann::json newCacheBakedFile;
      const AssetBakeCachedItem::BakedFile& bakedFile = newCacheItem.bakedFiles[i];
      newCacheBakedFile[cacheJsonStrings.filePath] = bakedFile.path;
      newCacheBakedFile[cacheJsonStrings.fileName] = bakedFile.name;
      newCacheBakedFile[cacheJsonStrings.fileExt] = bakedFile.ext;
      newCacheBakedFiles.push_back(newCacheBakedFile);
    }
    newCacheItemJson[cacheJsonStrings.bakedFiles] = newCacheBakedFiles;
    bakedFiles.push_back(newCacheItemJson);
  }

  cacheJson[cacheJsonStrings.cacheFiles] = bakedFiles;
  std::string jsonString = cacheJson.dump(1);
  writeFile(cacheJsonStrings.assetBakerCacheFileName, jsonString);
}

void loadCache(std::unordered_map<std::string, AssetBakeCachedItem>& assetBakeCache) {
  std::vector<char> fileBytes;

  if(!readFile(cacheJsonStrings.assetBakerCacheFileName, fileBytes)) {
    return;
  }

  std::string fileString(fileBytes.begin(), fileBytes.end());
  nlohmann::json cache = nlohmann::json::parse(fileString);

  nlohmann::json cachedFiles = cache[cacheJsonStrings.cacheFiles];

  for (auto& element : cachedFiles) {
    AssetBakeCachedItem cachedItem;
    cachedItem.originalFileName = element[cacheJsonStrings.originalFileName];
    cachedItem.originalFileLastModified = element[cacheJsonStrings.originalFileLastModified];
    u32 bakedFileCount = (u32)element[cacheJsonStrings.bakedFiles].size();
    for(u32 i = 0; i < bakedFileCount; i++) {
      nlohmann::json bakedFileJson = element[cacheJsonStrings.bakedFiles][i];
      AssetBakeCachedItem::BakedFile bakedFile;
      bakedFile.path = bakedFileJson[cacheJsonStrings.filePath];
      bakedFile.name = bakedFileJson[cacheJsonStrings.fileName];
      bakedFile.ext = bakedFileJson[cacheJsonStrings.fileExt];
      cachedItem.bakedFiles.push_back(bakedFile);
    }
    assetBakeCache[cachedItem.originalFileName] = cachedItem;
  }
}

int main(int argc, char* argv[]) {

  // NOTE: Count is often at least 1, as argv[0] is full path of the program being run
  if(argc < 3) {
    std::cout << "You need to supply the assets directory";
    return -1;
  }

  std::unordered_map<std::string, AssetBakeCachedItem> oldAssetBakeCache;
  loadCache(oldAssetBakeCache);
  std::vector<AssetBakeCachedItem> newlyCachedItems;
  newlyCachedItems.reserve(50);

  ConverterState converterState;
  converterState.assetsDir = {argv[1]};
  converterState.bakedAssetDir = converterState.assetsDir / "assets_export";
  converterState.outputFileDir = {argv[2]};

  if(!fs::is_directory(converterState.assetsDir)) {
    std::cout << "Could not find assets directory: " << argv[1];
    return -1;
  }

  // Create export folder if needed
  if(!fs::is_directory(converterState.bakedAssetDir)) {
    fs::create_directory(converterState.bakedAssetDir);
  }

  std::cout << "loaded asset directory at " << converterState.assetsDir << std::endl;

  fs::path asset_models_dir = converterState.assetsDir / "models";
  fs::path asset_skyboxes_dir = converterState.assetsDir / "skyboxes";
  fs::path asset_textures_dir = converterState.assetsDir / "textures";
  size_t fileCount = dirCountInDir(asset_skyboxes_dir);
  printf("skybox directories found: %d\n", (int)fileCount);

  std::filesystem::create_directory(converterState.bakedAssetDir / "models");
  std::filesystem::create_directory(converterState.bakedAssetDir / "skyboxes");
  std::filesystem::create_directory(converterState.bakedAssetDir / "textures");
  for(auto const& skyboxDir: std::filesystem::directory_iterator(asset_skyboxes_dir)) {
    if(fileUpToDate(oldAssetBakeCache, skyboxDir)) {
      continue;
    } else if(fs::is_directory(skyboxDir)) {
      printf("%s\n", skyboxDir.path().string().c_str());
      convertCubeMapTexture(skyboxDir, &converterState);
    }
  }

  /*
  size_t fileCount = fileCountInDir(converterState.assetsDir);
  converterState.bakedFilePaths.reserve(fileCount * 4);
  for(const fs::directory_entry& p: fs::directory_iterator(converterState.assetsDir)) { //fs::recursive_directory_iterator(directory)) {

    fs::path filePath = p.path();
    fs::path fileExt = filePath.extension();
    std::string pathStr = filePath.string();

    // skip directories and up-to-date baked assets
    if(fs::is_directory(filePath) || fileUpToDate(oldAssetBakeCache, filePath)) {
      continue;
    }

    std::cout << "File: " << pathStr << std::endl;

    u32 convertedFilesCountBefore = (u32)converterState.bakedFilePaths.size();

    if(fileExt == supportedFileExtensions.png || fileExt == supportedFileExtensions.jpg || fileExt == supportedFileExtensions.tga) {
      convertImage(p.path(), converterState);
    }
    else if(fileExt == supportedFileExtensions.obj) {
      std::cout << "OBJ: " << filePath.string() << std::endl;

      // find directory of file
      fs::path materialSearchPath = filePath.parent_path();
      assert(fs::is_directory(materialSearchPath));

      tinyobj::ObjReaderConfig readerConfig;
      readerConfig.mtl_search_path = materialSearchPath.string();

      tinyobj::ObjReader reader;
      if(!reader.ParseFromFile(filePath.string(), readerConfig)) {
        if(!reader.Error().empty()) {
          std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
      }

      if(!reader.Warning().empty()) {
        std::cout << "WARN (tinyobjloader): " << reader.Warning();
      }

      fs::path outputFolder = converterState.bakedAssetDir / (filePath.stem().string() + "_OBJ");
      fs::create_directory(outputFolder);

      extractObjCombinedMesh(reader, filePath, outputFolder, converterState);
    }
    else if(fileExt == supportedFileExtensions.gltf || fileExt == supportedFileExtensions.glb) {
      using namespace tinygltf;
      Model model;
      TinyGLTF loader;
      std::string err;
      std::string warn;

      bool ret;
      if(fileExt == supportedFileExtensions.gltf) {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filePath.string());
      } else { // glbExtension
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filePath.string());
      }

      if(!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
      }

      if(!err.empty()) {
        printf("Err: %s\n", err.c_str());
      }

      if(!ret) {
        printf("Failed to parse glTF\n");
        return -1;
      } else {
        fs::path outputFolder = converterState.bakedAssetDir / (p.path().stem().string() + "_GLTF");
        fs::create_directory(outputFolder);

        extractGltfCombinedMesh(model, filePath, outputFolder, converterState);
        extractGltfMaterials(model, filePath, outputFolder, converterState);
//        extractGltfMeshes(model, pathStr, outputFolder, converterState);
//        extractGltfNodes(model, filePath, outputFolder, converterState);
      }
    } else {
      continue;
    }

    // remember baked item
    AssetBakeCachedItem newlyBakedItem;
    newlyBakedItem.originalFileName = filePath.filename().string();
    newlyBakedItem.originalFileLastModified = lastModifiedTimeStamp(filePath);
    const fs::path& recentlyBakedFile = converterState.bakedFilePaths.back();
    u32 convertedFilesCount = (u32)converterState.bakedFilePaths.size();
    u32 newlyConvertedItemCount = convertedFilesCount - convertedFilesCountBefore;
    for(u32 i = 0; i < newlyConvertedItemCount; i++) {
      const fs::path& recentlyBakedFile = converterState.bakedFilePaths[convertedFilesCount - i - 1];
      AssetBakeCachedItem::BakedFile bakedFile;
      bakedFile.path = recentlyBakedFile.string();
      bakedFile.name = recentlyBakedFile.filename().string();
      bakedFile.ext = recentlyBakedFile.extension().string();
      newlyBakedItem.bakedFiles.push_back(bakedFile);
    }
    newlyCachedItems.push_back(newlyBakedItem);
  }

  writeOutputData(oldAssetBakeCache, converterState);
  saveCache(oldAssetBakeCache, newlyCachedItems);
  */

  return 0;
}

bool CompressionCallback(float fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2) {
  std::printf("\rCompression progress = %3.0f  ", fProgress);
  bool abortCompression = false;
  return abortCompression;
}

bool convertCubeMapTexture(const fs::path& inputDir, ConverterState* converterState) {

  int frontWidth, frontHeight, frontChannels,
      backWidth, backHeight, backChannels,
      topWidth, topHeight, topChannels,
      bottomWidth, bottomHeight, bottomChannels,
      leftWidth, leftHeight, leftChannels,
      rightWidth, rightHeight, rightChannels;

  fs::path ext;
  for(auto const& skyboxFaceImage: std::filesystem::directory_iterator(inputDir)) {
    if(fs::is_regular_file(skyboxFaceImage)) {
      ext = skyboxFaceImage.path().extension(); // pull extension from first file we find
      printf("%s\n", ext.string().c_str());
      break;
    }
  }

  if(ext.empty()) {
    printf("Skybox directory (%s) was found empty.", inputDir.string().c_str());
  }

  fs::path frontPath = (inputDir / "front").replace_extension(ext);
  fs::path backPath = (inputDir / "back").replace_extension(ext);
  fs::path topPath = (inputDir / "top").replace_extension(ext);
  fs::path bottomPath = (inputDir / "bottom").replace_extension(ext);
  fs::path leftPath = (inputDir / "left").replace_extension(ext);
  fs::path rightPath = (inputDir / "right").replace_extension(ext);

  auto imageLoadStart = std::chrono::high_resolution_clock::now();
  stbi_uc* frontPixels = stbi_load(frontPath.u8string().c_str(), &frontWidth, &frontHeight, &frontChannels, STBI_rgb);
  stbi_uc* backPixels = stbi_load(backPath.u8string().c_str(), &backWidth, &backHeight, &backChannels, STBI_rgb);
  stbi_uc* topPixels = stbi_load(topPath.u8string().c_str(), &topWidth, &topHeight, &topChannels, STBI_rgb);
  stbi_uc* bottomPixels = stbi_load(bottomPath.u8string().c_str(), &bottomWidth, &bottomHeight, &bottomChannels, STBI_rgb);
  stbi_uc* rightPixels = stbi_load(rightPath.u8string().c_str(), &rightWidth, &rightHeight, &rightChannels, STBI_rgb);
  stbi_uc* leftPixels = stbi_load(leftPath.u8string().c_str(), &leftWidth, &leftHeight, &leftChannels, STBI_rgb);
  auto imageLoadEnd = std::chrono::high_resolution_clock::now();

  auto diff = imageLoadEnd - imageLoadStart;
  std::cout << "srcTexture took " << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0 << "ms to load" << std::endl;

  if(!frontPixels || !backPixels || !topPixels || !bottomPixels || !leftPixels || !rightPixels) {
    std::cout << "Failed to load CubeMap face file for directory " << inputDir << std::endl;
    return false;
  }

  if(frontWidth != backWidth || frontWidth != topWidth || frontWidth != bottomWidth || frontWidth != leftWidth || frontWidth != rightWidth ||
      frontHeight != backHeight || frontHeight != topHeight || frontHeight != bottomHeight || frontHeight != leftHeight || frontHeight != rightHeight ||
      frontChannels != backChannels || frontChannels != topChannels || frontChannels != bottomChannels || frontChannels != leftChannels || frontChannels != rightChannels) {
    std::cout << "One or more CubeMap faces do not match in either width, height, or number of channels for directory " << inputDir << std::endl;
    return false;
  }

  if((frontWidth % 4) != 0 || (frontHeight % 4) != 0) {
    std::cout << "CubeMap face widths and heights must be evenly divisible by 4." << inputDir << std::endl;
    return false;
  }

  assert(topChannels == 3 && "Number of channels for cube map asset does not match desired");

  u32 facePixelsSize = frontWidth * frontHeight * frontChannels;
  unsigned char* cubeMapPixels_fbtbrl = (unsigned char*)malloc(facePixelsSize * 6);
  memcpy(cubeMapPixels_fbtbrl, frontPixels, facePixelsSize);
  memcpy(cubeMapPixels_fbtbrl + (1 * facePixelsSize), backPixels, facePixelsSize);
  memcpy(cubeMapPixels_fbtbrl + (2 * facePixelsSize), topPixels, facePixelsSize);
  memcpy(cubeMapPixels_fbtbrl + (3 * facePixelsSize), bottomPixels, facePixelsSize);
  memcpy(cubeMapPixels_fbtbrl + (4 * facePixelsSize), rightPixels, facePixelsSize);
  memcpy(cubeMapPixels_fbtbrl + (5 * facePixelsSize), leftPixels, facePixelsSize);

  stbi_image_free(frontPixels);
  stbi_image_free(backPixels);
  stbi_image_free(topPixels);
  stbi_image_free(bottomPixels);
  stbi_image_free(leftPixels);
  stbi_image_free(rightPixels);

  CMP_Texture srcTexture;
  srcTexture.dwSize = sizeof(srcTexture);
  srcTexture.dwWidth = topWidth;
  srcTexture.dwHeight = topHeight * 6;
  srcTexture.dwPitch = topWidth * topChannels;
  srcTexture.format = CMP_FORMAT_RGB_888;
  srcTexture.transcodeFormat = CMP_FORMAT_Unknown; // Should not be used
  srcTexture.nBlockHeight = 4;
  srcTexture.nBlockWidth = 4;
  srcTexture.nBlockDepth = 1;
  srcTexture.dwDataSize = topWidth * topHeight * topChannels * topChannels * 6;
  srcTexture.pData = cubeMapPixels_fbtbrl;
  srcTexture.pMipSet = nullptr;

  CMP_Texture destTexture;
  destTexture.dwSize     = sizeof(destTexture);
  destTexture.dwWidth    = srcTexture.dwWidth;
  destTexture.dwHeight   = srcTexture.dwHeight;
  destTexture.dwPitch    = 0;
  destTexture.format     = CMP_FORMAT_ETC2_RGB;
  destTexture.dwDataSize = CMP_CalculateBufferSize(&destTexture);
  destTexture.pData      = (CMP_BYTE*)malloc(destTexture.dwDataSize);

  CMP_CompressOptions options = {0};
  options.dwSize       = sizeof(options);
  options.fquality     = 1.0f;            // Quality
  options.dwnumThreads = 0;               // Number of threads to use per texture set to auto

  CubeMapInfo info;
  info.format = CubeMapFormat::ETC2_RGB;
  info.faceWidth = topWidth;
  info.faceHeight = topHeight;
  info.originalFolder = inputDir.string();
  info.faceSize = (u32)(ceil(info.faceWidth / 4.f) * ceil(info.faceHeight / 4.f)) * 8;

  auto compressionStart = std::chrono::high_resolution_clock::now();
  try {
    CMP_ERROR cmp_status = CMP_ConvertTexture(&srcTexture, &destTexture, &options, &CompressionCallback);
    if(cmp_status != CMP_OK) {
      std::printf("Error: Something went wrong with compressing %s\n", info.originalFolder.c_str());
    }
  } catch (const std::exception& ex) {
    std::printf("Error: %s\n",ex.what());
  }
  auto compressionEnd = std::chrono::high_resolution_clock::now();
  diff = compressionEnd - compressionStart;
  std::cout << "compression took " << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0 << "ms" << std::endl;

  assets::AssetFile cubeMapAssetFile = assets::packCubeMap(&info, destTexture.pData);

  free(cubeMapPixels_fbtbrl);
  free(destTexture.pData);

  fs::path relative = inputDir.lexically_proximate(converterState->assetsDir);
  fs::path exportPath = converterState->bakedAssetDir / relative;
  exportPath.replace_extension(bakedExtensions.cubeMap);

  saveAssetFile(exportPath.string().c_str(), cubeMapAssetFile);
  converterState->bakedFilePaths.push_back(exportPath);

  return true;
}

//bool convertImage(const fs::path& inputPath, ConverterState& converterState) {
//  int texWidth, texHeight, texChannels;
//
//  auto imageLoadStart = std::chrono::high_resolution_clock::now();
//
//  stbi_uc* pixels = stbi_load(inputPath.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//
//  auto imageLoadEnd = std::chrono::high_resolution_clock::now();
//
//  auto diff = imageLoadEnd - imageLoadStart;
//
//  std::cout << "texture took " << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0 << "ms to load" << std::endl;
//
//  if(!pixels) {
//    std::cout << "Failed to load texture file " << inputPath << std::endl;
//    return false;
//  }
//
//  TextureInfo texInfo;
//  texInfo.textureSize = texWidth * texHeight * 4;
//  texInfo.textureFormat = TextureFormat::RGBA8;
//  texInfo.originalFile = inputPath.string();
//  texInfo.width = texWidth;
//  texInfo.height = texHeight;
//
//  auto compressionStart = std::chrono::high_resolution_clock::now();
//  assets::AssetFile newImage = assets::packTexture(&texInfo, pixels);
//  auto compressionEnd = std::chrono::high_resolution_clock::now();
//
//  diff = compressionEnd - compressionStart;
//
//  std::cout << "compression took " << std::chrono::duration_cast<std::chrono::nanoseconds>(diff).count() / 1000000.0 << "ms" << std::endl;
//
//  stbi_image_free(pixels);
//
//  fs::path relative = inputPath.lexically_proximate(converterState.assetsDir);
//  fs::path exportPath = converterState.bakedAssetDir / relative;
//  exportPath.replace_extension(bakedExtensions.texture);
//
//  saveAssetFile(exportPath.string().c_str(), newImage);
//  converterState.bakedFilePaths.push_back(exportPath);
//
//  return true;
//}
//
//void packVertex(assets::Vertex_PNCV_f32& new_vert, tinyobj::real_t vx, tinyobj::real_t vy, tinyobj::real_t vz, tinyobj::real_t nx, tinyobj::real_t ny, tinyobj::real_t nz, tinyobj::real_t ux, tinyobj::real_t uy) {
//  new_vert.position[0] = vx;
//  new_vert.position[1] = vy;
//  new_vert.position[2] = vz;
//
//  new_vert.normal[0] = nx;
//  new_vert.normal[1] = ny;
//  new_vert.normal[2] = nz;
//
//  new_vert.uv[0] = ux;
//  new_vert.uv[1] = 1 - uy;
//}
//
//void packVertex(assets::Vertex_P32N8C8V16& new_vert, tinyobj::real_t vx, tinyobj::real_t vy, tinyobj::real_t vz, tinyobj::real_t nx, tinyobj::real_t ny, tinyobj::real_t nz, tinyobj::real_t ux, tinyobj::real_t uy) {
//  new_vert.position[0] = vx;
//  new_vert.position[1] = vy;
//  new_vert.position[2] = vz;
//
//  new_vert.normal[0] = u8(((nx + 1.0) / 2.0) * 255);
//  new_vert.normal[1] = u8(((ny + 1.0) / 2.0) * 255);
//  new_vert.normal[2] = u8(((nz + 1.0) / 2.0) * 255);
//
//  new_vert.uv[0] = ux;
//  new_vert.uv[1] = 1 - uy;
//}
//
//void unpackGltfBuffer(tinygltf::Model& model, tinygltf::Accessor& accessor, std::vector<u8>& outputBuffer) {
//  int bufferID = accessor.bufferView;
//  size_t elementSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
//
//  tinygltf::BufferView& bufferView = model.bufferViews[bufferID];
//
//  tinygltf::Buffer& bufferData = (model.buffers[bufferView.buffer]);
//
//
//  u8* dataptr = bufferData.data.data() + accessor.byteOffset + bufferView.byteOffset;
//
//  int components = tinygltf::GetNumComponentsInType(accessor.type);
//
//  elementSize *= components;
//
//  size_t stride = bufferView.byteStride;
//  if(stride == 0) {
//    stride = elementSize;
//
//  }
//
//  outputBuffer.resize(accessor.count * elementSize);
//
//  for(int i = 0; i < accessor.count; i++) {
//    u8* dataindex = dataptr + stride * i;
//    u8* targetptr = outputBuffer.data() + elementSize * i;
//
//    memcpy(targetptr, dataindex, elementSize);
//  }
//}
//
//void extractGltfVertices(tinygltf::Primitive& primitive, tinygltf::Model& model, std::vector<Vertex_PNCV_f32>& _vertices) {
//
//  tinygltf::Accessor& pos_accesor = model.accessors[primitive.attributes["POSITION"]];
//
//  _vertices.resize(pos_accesor.count);
//
//  std::vector<u8> pos_data;
//  unpackGltfBuffer(model, pos_accesor, pos_data);
//
//  u32 vertexCount = (u32)_vertices.size();
//  for(u32 i = 0; i < vertexCount; i++) {
//    if(pos_accesor.type == TINYGLTF_TYPE_VEC3) {
//      if(pos_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
//        f32* dtf = (f32*)pos_data.data();
//
//        //vec3f
//        _vertices[i].position[0] = *(dtf + (i * 3) + 0);
//        _vertices[i].position[1] = *(dtf + (i * 3) + 1);
//        _vertices[i].position[2] = *(dtf + (i * 3) + 2);
//      } else {
//        assert(false);
//      }
//    } else {
//      assert(false);
//    }
//  }
//
//  tinygltf::Accessor& normal_accesor = model.accessors[primitive.attributes["NORMAL"]];
//
//  std::vector<u8> normal_data;
//  unpackGltfBuffer(model, normal_accesor, normal_data);
//
//
//  for(u32 i = 0; i < vertexCount; i++) {
//    if(normal_accesor.type == TINYGLTF_TYPE_VEC3) {
//      if(normal_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
//        f32* dtf = (f32*)normal_data.data();
//
//        //vec3f
//        _vertices[i].normal[0] = *(dtf + (i * 3) + 0);
//        _vertices[i].normal[1] = *(dtf + (i * 3) + 1);
//        _vertices[i].normal[2] = *(dtf + (i * 3) + 2);
//
//        _vertices[i].color[0] = *(dtf + (i * 3) + 0);
//        _vertices[i].color[1] = *(dtf + (i * 3) + 1);
//        _vertices[i].color[2] = *(dtf + (i * 3) + 2);
//      } else {
//        assert(false);
//      }
//    } else {
//      assert(false);
//    }
//  }
//
//  tinygltf::Accessor& uv_accesor = model.accessors[primitive.attributes["TEXCOORD_0"]];
//
//  std::vector<u8> uv_data;
//  unpackGltfBuffer(model, uv_accesor, uv_data);
//
//
//  for(u32 i = 0; i < vertexCount; i++) {
//    if(uv_accesor.type == TINYGLTF_TYPE_VEC2) {
//      if(uv_accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
//        f32* dtf = (f32*)uv_data.data();
//
//        //vec3f
//        _vertices[i].uv[0] = *(dtf + (i * 2) + 0);
//        _vertices[i].uv[1] = *(dtf + (i * 2) + 1);
//      } else {
//        assert(false);
//      }
//    } else {
//      assert(false);
//    }
//  }
//
//  //for (auto& v : _vertices)
//  //{
//  //	v.position[0] *= -1;
//  //
//  //	v.normal[0] *= -1;
//  //	v.normal[1] *= -1;
//  //	v.normal[2] *= -1;
//  //	//v.position = flip * vec4(v.position, 1.f);
//  //}
//
//  return;
//}
//
//std::string calculateGltfMaterialName(tinygltf::Model& model, int materialIndex) {
//  char buffer[50];
//
//  intToStr(materialIndex, buffer);
//  std::string matname = "MAT_" + std::string{&buffer[0]} + "_" + model.materials[materialIndex].name;
//  return matname;
//}
//
//void extractGltfIndices(tinygltf::Primitive& primitive, tinygltf::Model& model, std::vector<u32>& _primIndices) {
//  int indexaccesor = primitive.indices;
//
//  int indexbuffer = model.accessors[indexaccesor].bufferView;
//  int componentType = model.accessors[indexaccesor].componentType;
//  size_t indexsize = tinygltf::GetComponentSizeInBytes(componentType);
//
//  tinygltf::BufferView& indexview = model.bufferViews[indexbuffer];
//  int bufferidx = indexview.buffer;
//
//  tinygltf::Buffer& buffindex = (model.buffers[bufferidx]);
//
//  u8* dataptr = buffindex.data.data() + indexview.byteOffset;
//
//  std::vector<u8> unpackedIndices;
//  unpackGltfBuffer(model, model.accessors[indexaccesor], unpackedIndices);
//
//  u32 gltfAccessorsCount = (u32)model.accessors[indexaccesor].count;
//  for(u32 i = 0; i < gltfAccessorsCount; i++) {
//
//    u32 index;
//    switch(componentType) {
//      case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT: {
//        u16* bfr = (u16*)unpackedIndices.data();
//        index = *(bfr + i);
//      }
//        break;
//      case TINYGLTF_COMPONENT_TYPE_SHORT: {
//        int16_t* bfr = (int16_t*)unpackedIndices.data();
//        index = *(bfr + i);
//      }
//        break;
//      default:
//        assert(false);
//    }
//
//    _primIndices.push_back(index);
//  }
//
//  u32 primIndicesCount = (u32)_primIndices.size() / 3;
//  for(u32 i = 0; i < primIndicesCount; i++) {
//    //flip the triangle
//
//    std::swap(_primIndices[i * 3 + 1], _primIndices[i * 3 + 2]);
//  }
//}
//
//std::string calculateGltfMeshName(tinygltf::Model& model, int meshIndex, int primitiveIndex) {
//  char buffer0[50];
//  char buffer1[50];
//  intToStr(meshIndex, buffer0);
//  intToStr(primitiveIndex, buffer1);
//
//  std::string meshname = "MESH_" + std::string{&buffer0[0]} + "_" + model.meshes[meshIndex].name;
//
//  bool multiprim = model.meshes[meshIndex].primitives.size() > 1;
//  if(multiprim) {
//    meshname += "_PRIM_" + std::string{&buffer1[0]};
//  }
//
//  return meshname;
//}
//
//bool extractGltfCombinedMesh(tinygltf::Model& gltfModel, const fs::path& filePath, const fs::path& outputFolder, ConverterState& converterState) {
//
//  using Vertex = assets::Vertex_PNCV_f32;
//  VertexFormat vertexFormat = assets::VertexFormat::PNCV_F32;
//
//  struct gltfAttributeMetadata {
//    s32 accessorIndex;
//    s32 numComponents;
//    s32 bufferViewIndex;
//    s32 bufferIndex;
//    size_t bufferByteOffset;
//    size_t bufferByteLength;
//  };
//
//  auto populateAttributeMetadata = [](const tinygltf::Model& model, const char* keyString, const tinygltf::Primitive& gltfPrimitive) -> gltfAttributeMetadata {
//    gltfAttributeMetadata result{};
//    result.accessorIndex = gltfPrimitive.attributes.at(keyString);
//    const tinygltf::Accessor& accessor = model.accessors[result.accessorIndex];
//    result.numComponents = tinygltf::GetNumComponentsInType(accessor.type);
//    result.bufferViewIndex = accessor.bufferView;
//    const tinygltf::BufferView& bufferView = model.bufferViews[result.bufferViewIndex];
//    result.bufferIndex = bufferView.buffer;
//    result.bufferByteOffset = bufferView.byteOffset;
//    result.bufferByteLength = bufferView.byteLength;
//    return result;
//  };
//
//  const char* positionAttrKeyString = "POSITION";
//  const char* normalAttrKeyString = "NORMAL";
//  const char* texture0AttrKeyString = "TEXCOORD_0";
//  const u32 positionAttributeIndex = 0;
//  const u32 normalAttributeIndex = 1;
//  const u32 texture0AttributeIndex = 2;
//
//  std::vector<Vertex> vertices;
//  std::vector<u32> indices;
//
//  u64 meshCount = gltfModel.meshes.size();
//  assert(meshCount > 0);
//  const std::vector<tinygltf::Accessor>& gltfAccessors = gltfModel.accessors;
//  const std::vector<tinygltf::BufferView>& gltfBufferViews = gltfModel.bufferViews;
//  const u64 bufferCount = gltfModel.buffers.size();
//
//  struct UniqueVert {
//    u32 positionBufferOffset;
//    u32 normalBufferOffset;
//    u32 texCoordBufferOffset;
//    s32 positionBufferIndex;
//    s32 normalBufferIndex;
//    s32 texCoordBufferIndex;
//    s32 materialIndex;
//  };
//
//  auto indexHashLambda = [=](const UniqueVert& uniqueVert) {
//    // Note: We're making assumptions that offsets are rarely over 2^21(~2'000'000)
//    u64 hashVal = uniqueVert.positionBufferOffset;
//    hashVal = (hashVal << 21);
//    hashVal += uniqueVert.normalBufferOffset;
//    hashVal = (hashVal << 21);
//    hashVal += uniqueVert.texCoordBufferOffset;
//    return hashVal;
//  };
//  auto indexEqualsLambda = [=](const UniqueVert& A, const UniqueVert& B) {
//    return A.positionBufferOffset == B.positionBufferOffset &&
//           A.normalBufferOffset == B.normalBufferOffset &&
//           A.texCoordBufferOffset == B.texCoordBufferOffset &&
//           A.positionBufferIndex == B.positionBufferIndex &&
//           A.normalBufferIndex == B.normalBufferIndex &&
//           A.texCoordBufferIndex == B.texCoordBufferIndex &&
//           A.materialIndex == B.materialIndex;
//  };
//  std::unordered_map<UniqueVert, u32 /*vertIndex*/, decltype(indexHashLambda), decltype(indexEqualsLambda)> cachedIndexValues(100'000, indexHashLambda, indexEqualsLambda);
//
//  for(u32 gltfMeshIndex = 0; gltfMeshIndex < meshCount; gltfMeshIndex++) {
//    tinygltf::Mesh& gltfMesh = gltfModel.meshes[gltfMeshIndex];
//    u64 primitiveCount = gltfMesh.primitives.size();
//    cachedIndexValues.clear(); // Not allowing vertices to be shared across meshes
//    for(u32 gltfPrimitiveIndex = 0; gltfPrimitiveIndex < primitiveCount; gltfPrimitiveIndex++) {
//      tinygltf::Primitive& gltfPrimitive = gltfMesh.primitives[gltfPrimitiveIndex];
//      assert(gltfPrimitive.indices > -1);
//
//      // indices data
//      u32 indicesAccessorIndex = gltfPrimitive.indices;
//      const tinygltf::Accessor& indicesAccessor = gltfModel.accessors[indicesAccessorIndex];
//      const tinygltf::BufferView& indicesGLTFBufferView = gltfBufferViews[indicesAccessor.bufferView];
//      u32 indicesGLTFBufferIndex = indicesGLTFBufferView.buffer;
//      u64 indicesGLTFBufferByteOffset = indicesGLTFBufferView.byteOffset;
//      u64 indicesGLTFBufferByteLength = indicesGLTFBufferView.byteLength;
//      u16* indexValues = (u16*)(gltfModel.buffers[indicesGLTFBufferIndex].data.data() + indicesGLTFBufferByteOffset);
//
//      // position attributes
//      assert(gltfPrimitive.attributes.find(positionAttrKeyString) != gltfPrimitive.attributes.end());
//      gltfAttributeMetadata positionAttribute = populateAttributeMetadata(gltfModel, positionAttrKeyString, gltfPrimitive);
//      f32* positionAttributeValues = (f32*)(gltfModel.buffers[positionAttribute.bufferIndex].data.data());
//      u32 positionAttributeOffset = (u32)(positionAttribute.bufferByteOffset / sizeof(f32));
//
//      // normal attributes
//      bool normalAttributesAvailable = gltfPrimitive.attributes.find(normalAttrKeyString) != gltfPrimitive.attributes.end();
//      assert(normalAttributesAvailable); // TODO: Calc normals if not available?
//      gltfAttributeMetadata normalAttribute = populateAttributeMetadata(gltfModel, normalAttrKeyString, gltfPrimitive);
//      f32* normalAttributeValues = (f32*)(gltfModel.buffers[normalAttribute.bufferIndex].data.data());
//      u32 normalAttributeOffset = (u32)(normalAttribute.bufferByteOffset / sizeof(f32));
//
//      // texture 0 uv coord attribute data
//      bool texture0AttributesAvailable = gltfPrimitive.attributes.find(texture0AttrKeyString) != gltfPrimitive.attributes.end();
//      gltfAttributeMetadata texture0Attribute{};
//      f32* texture0AttributeValues = nullptr;
//      u32 texture0AttributeOffset = 0;
//      if(texture0AttributesAvailable) {
//        texture0Attribute = populateAttributeMetadata(gltfModel, texture0AttrKeyString, gltfPrimitive);
//        texture0AttributeValues = (f32*)(gltfModel.buffers[texture0Attribute.bufferIndex].data.data());
//        texture0AttributeOffset = (u32)(texture0Attribute.bufferByteOffset / sizeof(f32));
//      }
//
//      f64 defaultBaseColor[] = {1.0, 1.0, 1.0, 1.0};
//      f64* baseColor = gltfPrimitive.material > 0 ?
//                       gltfModel.materials[gltfPrimitive.material].pbrMetallicRoughness.baseColorFactor.data():
//                       defaultBaseColor;
//
//      u64 indexCount = indicesGLTFBufferByteLength / sizeof(u16);
//      u64 vertexCount = positionAttribute.bufferByteLength / positionAttribute.numComponents / sizeof(f32);
//      for(u32 gltfVertexIndex = 0; gltfVertexIndex < indexCount; gltfVertexIndex++) {
//
//        u16 vertIndex = indexValues[gltfVertexIndex];
//
//        UniqueVert vertIndexInfo{};
//        vertIndexInfo.positionBufferOffset = positionAttributeOffset + (3 * vertIndex);
//        vertIndexInfo.normalBufferOffset = normalAttributeOffset + (3 * vertIndex);
//        vertIndexInfo.texCoordBufferOffset = texture0AttributeOffset + (2 * vertIndex);
//        vertIndexInfo.positionBufferIndex = positionAttribute.bufferIndex;
//        vertIndexInfo.normalBufferIndex = normalAttribute.bufferIndex;
//        vertIndexInfo.texCoordBufferIndex = texture0Attribute.bufferIndex;
//        vertIndexInfo.materialIndex = gltfPrimitive.material;
//
//        auto cachedVertex = cachedIndexValues.find(vertIndexInfo);
//        if(cachedVertex != cachedIndexValues.end()) {
//          indices.push_back(cachedVertex->second);
//          continue;
//        }
//
//        Vertex newVert{};
//
//        //vertex position
//        newVert.position[0] = positionAttributeValues[vertIndexInfo.positionBufferOffset + 0];
//        newVert.position[1] = positionAttributeValues[vertIndexInfo.positionBufferOffset + 1];
//        newVert.position[2] = positionAttributeValues[vertIndexInfo.positionBufferOffset + 2];
//
//        //vertex normal
//        newVert.normal[0] = normalAttributeValues[vertIndexInfo.normalBufferOffset + 0];
//        newVert.normal[1] = normalAttributeValues[vertIndexInfo.normalBufferOffset + 1];
//        newVert.normal[2] = normalAttributeValues[vertIndexInfo.normalBufferOffset + 2];
//
//        //vertex color
//        newVert.color[0] = (f32)baseColor[0];
//        newVert.color[1] = (f32)baseColor[1];
//        newVert.color[2] = (f32)baseColor[2];
//
//        // vertex uv
//        if(texture0AttributesAvailable) {
//          newVert.uv[0] = texture0AttributeValues[vertIndexInfo.texCoordBufferOffset + 0];
//          newVert.uv[1] = 1.0f - texture0AttributeValues[vertIndexInfo.texCoordBufferOffset + 1]; // TODO: is inverse uv y coord necessary?
//        } else {
//          newVert.uv[0] = 0.5f;
//          newVert.uv[1] = 0.5f;
//        }
//
//        vertices.push_back(newVert);
//        u32 newVertIndex = (u32)(vertices.size() - 1);
//        indices.push_back(newVertIndex);
//        std::pair<UniqueVert, u32> newCachedVertex = {vertIndexInfo, newVertIndex };
//        cachedIndexValues.insert(newCachedVertex);
//      }
//    }
//  }
//
//  MeshInfo meshInfo;
//  meshInfo.vertexFormat = vertexFormat;
//  meshInfo.vertexBufferSize = vertices.size() * sizeof(Vertex);
//  meshInfo.indexBufferSize = indices.size() * sizeof(u32);
//  meshInfo.indexSize = sizeof(u32);
//  meshInfo.originalFile = filePath.string();
//  meshInfo.bounds = assets::calculateBounds(vertices.data(), vertices.size());
//
//  assets::AssetFile newFile = assets::packMesh(meshInfo, (char*)vertices.data(), (char*)indices.data());
//
//  std::string newFileName = filePath.filename().replace_extension(bakedExtensions.mesh).string();
//  fs::path meshPath = outputFolder / newFileName;
//
//  //save to disk
//  saveAssetFile(meshPath.string().c_str(), newFile);
//  converterState.bakedFilePaths.push_back(meshPath);
//
//  return true;
//}
//
//bool extractGltfMeshes(tinygltf::Model& gltfModel, const std::string& filePath, const fs::path& outputFolder, ConverterState& converterState) {
//  for(auto meshIndex = 0; meshIndex < gltfModel.meshes.size(); meshIndex++) {
//
//    tinygltf::Mesh& gltfMesh = gltfModel.meshes[meshIndex];
//
//    using VertexFormat = assets::Vertex_PNCV_f32;
//    auto VertexFormatEnum = assets::VertexFormat::PNCV_F32;
//
//    std::vector<VertexFormat> vertices;
//    std::vector<u32> indices;
//
//    for(auto primitiveIndex = 0; primitiveIndex < gltfMesh.primitives.size(); primitiveIndex++) {
//
//      vertices.clear();
//      indices.clear();
//
//      std::string meshName = calculateGltfMeshName(gltfModel, meshIndex, primitiveIndex);
//
//      tinygltf::Primitive& primitive = gltfMesh.primitives[primitiveIndex];
//
//      extractGltfIndices(primitive, gltfModel, indices);
//      extractGltfVertices(primitive, gltfModel, vertices);
//
//      MeshInfo meshInfo;
//      meshInfo.vertexFormat = VertexFormatEnum;
//      meshInfo.vertexBufferSize = vertices.size() * sizeof(VertexFormat);
//      //meshInfo.indexBufferSize = indices.size() * sizeof(u32);
//      //meshInfo.indexSize = sizeof(u32);
//      meshInfo.originalFile = filePath;
//
//      meshInfo.bounds = assets::calculateBounds(vertices.data(), vertices.size());
//
//      assets::AssetFile newFile = assets::packMesh(meshInfo, (char*)vertices.data(), (char*)indices.data());
//
//      fs::path meshPath = outputFolder / (meshName + bakedExtensions.mesh);
//
//      //save to disk
//      saveAssetFile(meshPath.string().c_str(), newFile);
//      converterState.bakedFilePaths.push_back(meshPath);
//    }
//  }
//  return true;
//}
//
//void extractGltfMaterials(tinygltf::Model& model, const fs::path& input, const fs::path& outputFolder, ConverterState& converterState) {
//
//  int materialIndex = 0;
//  for(tinygltf::Material& gltfMat: model.materials) {
//    std::string matName = calculateGltfMaterialName(model, materialIndex++);
//
//    tinygltf::PbrMetallicRoughness& pbr = gltfMat.pbrMetallicRoughness;
//
//    assets::MaterialInfo newMaterial;
//    newMaterial.baseEffect = "defaultPBR";
//
//    if(pbr.baseColorTexture.index >= 0) {
//      tinygltf::Texture baseColor = model.textures[pbr.baseColorTexture.index];
//      tinygltf::Image baseImage = model.images[baseColor.source];
//
//      fs::path baseColorPath = outputFolder.parent_path() / baseImage.uri;
//
//      baseColorPath.replace_extension(bakedExtensions.texture);
//
//      baseColorPath = converterState.convertToExportRelative(baseColorPath);
//
//      newMaterial.textures["baseColor"] = baseColorPath.string();
//    }
//    if(pbr.metallicRoughnessTexture.index >= 0) {
//      tinygltf::Texture image = model.textures[pbr.metallicRoughnessTexture.index];
//      tinygltf::Image baseImage = model.images[image.source];
//
//      fs::path baseColorPath = outputFolder.parent_path() / baseImage.uri;
//
//      baseColorPath.replace_extension(bakedExtensions.texture);
//
//      baseColorPath = converterState.convertToExportRelative(baseColorPath);
//
//      newMaterial.textures["metallicRoughness"] = baseColorPath.string();
//    }
//
//    if(gltfMat.normalTexture.index >= 0) {
//      tinygltf::Texture image = model.textures[gltfMat.normalTexture.index];
//      tinygltf::Image baseImage = model.images[image.source];
//
//      fs::path baseColorPath = outputFolder.parent_path() / baseImage.uri;
//
//      baseColorPath.replace_extension(bakedExtensions.texture);
//
//      baseColorPath = converterState.convertToExportRelative(baseColorPath);
//
//      newMaterial.textures["normals"] = baseColorPath.string();
//    }
//
//    if(gltfMat.occlusionTexture.index >= 0) {
//      tinygltf::Texture image = model.textures[gltfMat.occlusionTexture.index];
//      tinygltf::Image baseImage = model.images[image.source];
//
//      fs::path baseColorPath = outputFolder.parent_path() / baseImage.uri;
//
//      baseColorPath.replace_extension(bakedExtensions.texture);
//
//      baseColorPath = converterState.convertToExportRelative(baseColorPath);
//
//      newMaterial.textures["occlusion"] = baseColorPath.string();
//    }
//
//    if(gltfMat.emissiveTexture.index >= 0) {
//      tinygltf::Texture image = model.textures[gltfMat.emissiveTexture.index];
//      tinygltf::Image baseImage = model.images[image.source];
//
//      fs::path baseColorPath = outputFolder.parent_path() / baseImage.uri;
//
//      baseColorPath.replace_extension(bakedExtensions.texture);
//
//      baseColorPath = converterState.convertToExportRelative(baseColorPath);
//
//      newMaterial.textures["emissive"] = baseColorPath.string();
//    }
//
//    fs::path materialPath = outputFolder / (matName + bakedExtensions.material);
//
//    if(gltfMat.alphaMode.compare("BLEND") == 0) {
//      newMaterial.transparency = TransparencyMode::Transparent;
//    } else {
//      newMaterial.transparency = TransparencyMode::Opaque;
//    }
//
//    assets::AssetFile newFile = assets::packMaterial(&newMaterial);
//
//    //save to disk
//    saveAssetFile(materialPath.string().c_str(), newFile);
//    converterState.bakedFilePaths.push_back(materialPath);
//  }
//}
//
//void extractGltfNodes(tinygltf::Model& model, const fs::path& input, const fs::path& outputFolder, ConverterState& converterState) {
//  assets::PrefabInfo prefab;
//
//  std::vector<u64> meshNodes;
//  u32 gltfNodeCount = (u32)model.nodes.size();
//  for(u32 i = 0; i < gltfNodeCount; i++) {
//    auto& node = model.nodes[i];
//    prefab.nodeNames[i] = node.name;
//
//    mat4 nodeMatrix;
//
//    mat4 flipY = mat4{ 1.0 };
//    flipY.val2d[1][1] = -1;
//
//    //node has a nodeMatrix
//    if(!node.matrix.empty()) {
//      for(u32 n = 0; n < 16; n++) {
//        nodeMatrix.val[n] = (f32)node.matrix[n];
//      }
//      //nodeMatrix = nodeMatrix * flipY;
//    } else { //separate transforms
//      mat4 translation{1.f};
//      if(!node.translation.empty()) {
//        translation = translate_mat4(vec3{(f32)node.translation[0],
//                                     (f32)node.translation[1],
//                                     (f32)node.translation[2]});
//      }
//
//      mat4 rotation{1.f};
//      if(!node.rotation.empty()) {
//        quaternion rot{(f32)node.rotation[3],
//                      (f32)node.rotation[0],
//                      (f32)node.rotation[1],
//                      (f32)node.rotation[2]};
//        rotation = rotate_mat4(rot);
//      }
//
//      mat4 scale{1.f};
//      if(!node.scale.empty()) {
//        scale = scale_mat4(vec3{(f32)node.scale[0],
//                                     (f32)node.scale[1],
//                                     (f32)node.scale[2]});
//      }
//
//      mat4 transformMatrix = (translation * rotation * scale);// * flipY;
//      memcpy(nodeMatrix.val, &transformMatrix, sizeof(mat4));
//    }
//
//    prefab.nodeMatrices[i] = (u32)prefab.matrices.size();
//    prefab.matrices.push_back(nodeMatrix);
//
//    if(node.mesh >= 0) {
//      auto mesh = model.meshes[node.mesh];
//
//      if(mesh.primitives.size() > 1) {
//        meshNodes.push_back(i);
//      } else {
//        auto primitive = mesh.primitives[0];
//        std::string meshName = calculateGltfMeshName(model, node.mesh, 0);
//
//        fs::path meshPath = outputFolder / (meshName + bakedExtensions.mesh);
//
//        int material = primitive.material;
//
//        std::string matName = calculateGltfMaterialName(model, material);
//
//        fs::path materialPath = outputFolder / (matName + bakedExtensions.material);
//
//        assets::PrefabInfo::NodeMesh nodeMesh;
//        nodeMesh.meshPath = converterState.convertToExportRelative(meshPath).string();
//        nodeMesh.materialPath = converterState.convertToExportRelative(materialPath).string();
//
//        prefab.nodeMeshes[i] = nodeMesh;
//      }
//    }
//  }
//
//  //calculate parent hierarchies
//  //gltf stores children, but we want parent
//  for(u32 i = 0; i < gltfNodeCount; i++) {
//    for(auto c: model.nodes[i].children) {
//      prefab.nodeParents[c] = i;
//    }
//  }
//
//  //for every gltf node that is a root node (no parents), apply the coordinate fixup
//
//  mat4 flip = mat4{1.0};
//  flip.val2d[1][1] = -1;
//
//
//  mat4 rotation = mat4{1.0};
//  //flip[1][1] = -1;
//  rotation = rotate_mat4(radians(-180.f), vec3{1, 0, 0});
//
//
//  //flip[2][2] = -1;
//  for(u32 i = 0; i < gltfNodeCount; i++) {
//    auto it = prefab.nodeParents.find(i);
//    if(it == prefab.nodeParents.end()) {
//      auto matrix = prefab.matrices[prefab.nodeMatrices[i]];
//      //no parent, root node
//      mat4 mat;
//      memcpy(&mat, &matrix, sizeof(mat4));
//
//      mat = rotation * (flip * mat);
//      memcpy(&matrix, &mat, sizeof(mat4));
//
//      prefab.matrices[prefab.nodeMatrices[i]] = matrix;
//    }
//  }
//
//  size_t nodeIndex = model.nodes.size();
//  //iterate nodes with mesh, convert each submesh into a node
//  u32 meshNodesCount = (u32)meshNodes.size();
//  for(u32 i = 0; i < meshNodesCount; i++) {
//    tinygltf::Node& node = model.nodes[i];
//
//    if(node.mesh < 0) break;
//
//    tinygltf::Mesh mesh = model.meshes[node.mesh];
//
//    for(int primindex = 0; primindex < mesh.primitives.size(); primindex++) {
//      tinygltf::Primitive primitive = mesh.primitives[primindex];
//
//      char buffer[50];
//
//      intToStr(primindex, buffer);
//
//      prefab.nodeNames[nodeIndex] = prefab.nodeNames[i] + "_PRIM_" + &buffer[0];
//
//      int material = primitive.material;
//      auto mat = model.materials[material];
//      std::string matName = calculateGltfMaterialName(model, material);
//      std::string meshName = calculateGltfMeshName(model, node.mesh, primindex);
//
//      fs::path materialPath = outputFolder / (matName + bakedExtensions.material);
//      fs::path meshPath = outputFolder / (meshName + bakedExtensions.mesh);
//
//      assets::PrefabInfo::NodeMesh nodeMesh;
//      nodeMesh.meshPath = converterState.convertToExportRelative(meshPath).string();
//      nodeMesh.materialPath = converterState.convertToExportRelative(materialPath).string();
//
//      prefab.nodeMeshes[nodeIndex] = nodeMesh;
//      nodeIndex++;
//    }
//  }
//
//
//  assets::AssetFile newFile = assets::packPrefab(prefab);
//
//  fs::path sceneFilePath = (outputFolder.parent_path()) / input.stem();
//
//  sceneFilePath.replace_extension(bakedExtensions.prefab);
//
//  //save to disk
//  saveAssetFile(sceneFilePath.string().c_str(), newFile);
//  converterState.bakedFilePaths.push_back(sceneFilePath);
//}
//
//bool extractObjCombinedMesh(tinyobj::ObjReader& objReader, const fs::path& filePath, const fs::path& outputFolder, ConverterState& converterState) {
//  using Vertex = assets::Vertex_PNCV_f32;
//  VertexFormat vertexFormat = assets::VertexFormat::PNCV_F32;
//
//  //attrib will contain the vertex arrays of the file
//  tinyobj::attrib_t attrib = objReader.GetAttrib();
//  u64 vertexPositionsCount = attrib.vertices.size() / 3;
//  u64 vertexColorsCount = attrib.colors.size() / 3;
//  u64 vertexNormalsCount = attrib.normals.size() / 3;
//  u64 vertexUVsCount = attrib.texcoords.size() / 3;
//  //shapes contains the info for each separate object in the file
//  std::vector<tinyobj::shape_t> shapes = objReader.GetShapes();
//  //materials contains the information about the material of each shape, but we won't use it.
//  std::vector<tinyobj::material_t> materials = objReader.GetMaterials();
//
//  u64 vertexCount = attrib.vertices.size() / 3;
//
//  std::vector<Vertex> vertices;
//  vertices.reserve(vertexCount * 2); // guesstimate
//
//  std::vector<u32> indices;
//  indices.reserve(vertexCount * 2); // guesstimate
//
//  auto tinyobjIndexHashLambda = [=](const tinyobj::index_t& tinyobjIndex) {
//    size_t hashVal = tinyobjIndex.vertex_index;
//    hashVal += (tinyobjIndex.texcoord_index * vertexPositionsCount);
//    hashVal += (tinyobjIndex.normal_index * (vertexPositionsCount * vertexUVsCount));
//    return hashVal;
//  };
//  auto tinyobjIndexEqualsLambda = [=](const tinyobj::index_t& A, const tinyobj::index_t& B) {
//    return A.vertex_index == B.vertex_index &&
//          A.normal_index == B.normal_index &&
//          A.texcoord_index == B.texcoord_index;
//  };
//  std::unordered_map<tinyobj::index_t, u32 /*vertIndex*/, decltype(tinyobjIndexHashLambda), decltype(tinyobjIndexEqualsLambda)> cachedIndexValues(vertexCount * 3, tinyobjIndexHashLambda, tinyobjIndexEqualsLambda);
//
//  for(u64 shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++) {
//    const tinyobj::shape_t& shape = shapes[shapeIndex];
//    assert(shape.lines.indices.size() == 0); // assert no lines
//    assert(shape.points.indices.size() == 0); // assert no points
//    const tinyobj::mesh_t& mesh = shape.mesh;
//    u64 meshFaceCount = mesh.num_face_vertices.size();
//    size_t meshIndexOffset = 0;
//
//    for(u64 faceIndex = 0; faceIndex < meshFaceCount; faceIndex++) {
//      u8 faceVertexCount = mesh.num_face_vertices[faceIndex];
//      assert(faceVertexCount == 3); // NOTE: Currently an error if dealing with non-triangles
//
//      // Loop over vertices in the face.
//      for(u32 faceVertIndex = 0; faceVertIndex < faceVertexCount; faceVertIndex++) {
//        // access to vertex
//        tinyobj::index_t tinyobjIndex = mesh.indices[meshIndexOffset + faceVertIndex];
//        s32 tinyobj_vertexIndex = tinyobjIndex.vertex_index;
//        s32 tinyobj_normalIndex = tinyobjIndex.normal_index;
//        s32 tinyobj_texCoordIndex = tinyobjIndex.texcoord_index;
//        assert(tinyobj_vertexIndex < vertexCount);
//        assert(tinyobj_vertexIndex >= 0);
//
//        auto cachedIndexValue = cachedIndexValues.find(tinyobjIndex);
//
//        s64 matchedVertIndex = -1;
//        if(cachedIndexValue != cachedIndexValues.end()) {
//          matchedVertIndex = cachedIndexValue->second;
//        }
//
//        if(matchedVertIndex != -1) {
//          indices.push_back((u32)matchedVertIndex);
//        } else {// new vertex
//
//          Vertex newVert;
//
//          //vertex position
//          u32 vertexAttributeStartIndex = 3 * tinyobjIndex.vertex_index;
//          newVert.position[0] = attrib.vertices[vertexAttributeStartIndex + 0];
//          newVert.position[1] = attrib.vertices[vertexAttributeStartIndex + 1];
//          newVert.position[2] = attrib.vertices[vertexAttributeStartIndex + 2];
//
//          //vertex normal
//          if(vertexNormalsCount > 0) {
//            u32 normalAttributeStartIndex = 3 * tinyobjIndex.normal_index;
//            newVert.normal[0] = attrib.normals[normalAttributeStartIndex + 0];
//            newVert.normal[1] = attrib.normals[normalAttributeStartIndex + 1];
//            newVert.normal[2] = attrib.normals[normalAttributeStartIndex + 2];
//          } else {
//            // TODO: calculate normals
//            // NOTE: reference
//            // tinyobjloader/tinyobjloader/blob/master/examples/viewer/viewer.cc : computeSmoothingNormals()
//            printf("OBJ file %s is missing normals", filePath.filename().string().c_str());
//          }
//
//          // Optional: vertex colors
//          if(vertexColorsCount > 0) {
//            u32 colorAttributeStartIndex = vertexAttributeStartIndex;
//            newVert.color[0] = attrib.colors[colorAttributeStartIndex + 0];
//            newVert.color[1] = attrib.colors[colorAttributeStartIndex + 1];
//            newVert.color[2] = attrib.colors[colorAttributeStartIndex + 2];
//          } else { // set vertex colors to normals
//            newVert.color[0] = newVert.normal[0];
//            newVert.color[1] = newVert.normal[1];
//            newVert.color[2] = newVert.normal[2];
//          }
//
//          //vertex uv
//          if(vertexUVsCount > 0) {
//            u32 texCoordAttributeStartIndex = 2 * tinyobjIndex.texcoord_index;
//            newVert.uv[0] = attrib.texcoords[texCoordAttributeStartIndex + 0];
//            newVert.uv[1] = 1.0f - attrib.texcoords[texCoordAttributeStartIndex + 1];
//          } else {
//            newVert.uv[0] = 0.5f;
//            newVert.uv[1] = 0.5f;
//          }
//
//          vertices.push_back(newVert);
//          u32 newVertIndex = (u32)(vertices.size() - 1);
//          indices.push_back(newVertIndex);
//
//          std::pair<tinyobj::index_t, u32 /*vertIndex*/> newCachedValue = { tinyobjIndex, newVertIndex };
//          cachedIndexValues.insert(newCachedValue);
//        }
//      }
//      meshIndexOffset += faceVertexCount;
//    }
//  }
//
//  MeshInfo meshInfo;
//  meshInfo.vertexFormat = vertexFormat;
//  meshInfo.vertexBufferSize = vertices.size() * sizeof(Vertex);
//  meshInfo.indexBufferSize = indices.size() * sizeof(u32);
//  meshInfo.indexSize = sizeof(u32);
//  meshInfo.originalFile = filePath.string();
//  meshInfo.bounds = assets::calculateBounds(vertices.data(), vertices.size());
//
//  assets::AssetFile newFile = assets::packMesh(meshInfo, (char*)vertices.data(), (char*)indices.data());
//
//  std::string newFileName = filePath.filename().replace_extension(bakedExtensions.mesh).string();
//  fs::path meshPath = outputFolder / newFileName;
//
//  //save to disk
//  saveAssetFile(meshPath.string().c_str(), newFile);
//  converterState.bakedFilePaths.push_back(meshPath);
//
//  return true;
//}