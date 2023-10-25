#include <cstdarg>
#include <iostream>
#include <unordered_set>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include "lz4/lz4.h"
#include "nlohmann/json.hpp"
#include "compressonator.h"

#include "stb/stb_image.h"
#include "tinyobjloader/tiny_obj_loader.h"
#include "tinygltf/tiny_gltf.h"

#include "noop_types.h"

#include "util.cpp"

#include "asset_loader.h"
#include "texture_asset.h"
#include "cubemap_asset.h"
#include "model_asset.h"
using namespace assets;

#include "noop_math.h"
using namespace noop;

#define assert_release(expression) ((void)0)

struct {
  const char* texture = ".tx";
  const char* cubeMap = ".cbtx";
  const char* model = ".modl";
} bakedExtensions;

const char* assetBakerCacheFileName = "Asset-Baker-Cache.asb";
struct {
  const char* cacheFiles = "cacheFiles";
  const char* originalFileName = "originalFileName";
  const char* originalFileLastModified = "originalFileLastModified";
  const char* bakedFiles = "bakedFiles";
  const char* fileName = "fileName";
  const char* filePath = "filePath";
} cacheJsonStrings;

struct ConverterState {
  fs::path assetsDir;
  fs::path bakedAssetDir;
  fs::path outputFileDir;
  std::vector<fs::path> bakedFilePaths;
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

bool convertTexture(const fs::path& inputPath, const char* outputFilename);
bool convertCubeMapTexture(const fs::path& inputDir, const char* outputFilename);
bool convertModel(const fs::path& inputPath, const char* outputFileName);

void saveCache(const std::unordered_map<std::string, AssetBakeCachedItem>& oldCache, const std::vector<AssetBakeCachedItem>& newBakedItems);
void loadCache(std::unordered_map<std::string, AssetBakeCachedItem>& assetBakeCache);

void writeOutputData(const std::unordered_map<std::string, AssetBakeCachedItem>& oldCache, const ConverterState& converterState);
void replace(std::string& str, const char oldToken, const char newToken);
void replaceBackSlashes(std::string& str);
std::size_t fileCountInDir(const fs::path& dirPath);
std::size_t dirCountInDir(const fs::path& dirPath);

f64 lastModifiedTimeStamp(const fs::path& file);
bool fileUpToDate(const std::unordered_map<std::string, AssetBakeCachedItem>& cache, const fs::path& file);
void replace(std::string& str, const char* oldTokens, u32 oldTokensCount, char newToken);

bool compressImage(u8* uncompressedBytes, u32 width, u32 height, u32 numChannels, u8* compressedBytes);
bool CompressionCallback(float fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2);

bool bakeFailed = false;

const char* rawAssetsDir = "native_scenes/src/main/assets_raw";
const char* bakedAssetsDir = "native_scenes/src/main/assets";
const char* metadataOutputDir = "native_scenes/src/main/cpp/assets_metadata";

void outputErrorMsg(const char* format, ...) {
  va_list args;
  va_start(args, format);
  fprintf(stderr, format, args);
  va_end(args);

  bakeFailed = true;
}

int main(int argc, char* argv[]) {
  // NOTE: Count is often at least 1, as argv[0] is full path of the program being run
  if(argc > 1) {
    char* arg1 = {argv[1]};
    if(strcmp(arg1, "--clean") == 0) {
      fs::path cacheFile{assetBakerCacheFileName};
      if(fs::remove(cacheFile)) {
        printf("Successfully deleted cache.");
      } else {
        printf("Attempted to clean but cache file was not found.");
      }
      return 0;
    }

    outputErrorMsg("Unsupported options.\n");
    outputErrorMsg("Use ex: .\\assetbaker {--clean}\n");
    return -1;
  }

  std::unordered_map<std::string, AssetBakeCachedItem> oldAssetBakeCache;
  loadCache(oldAssetBakeCache);
  std::vector<AssetBakeCachedItem> newlyCachedItems;
  newlyCachedItems.reserve(50);

  ConverterState converterState;
  converterState.assetsDir = rawAssetsDir;
  converterState.bakedAssetDir = bakedAssetsDir;
  converterState.outputFileDir = metadataOutputDir;

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
  fs::create_directory(converterState.bakedAssetDir / "models");
  fs::create_directory(converterState.bakedAssetDir / "skyboxes");
  fs::create_directory(converterState.bakedAssetDir / "textures");

  // TODO: Bring back with a cache that respects file formats
  size_t skyboxDirCount = dirCountInDir(asset_skyboxes_dir);
  printf("skybox directories found: %d\n", (int)skyboxDirCount);
  for(auto const& skyboxDir: std::filesystem::directory_iterator(asset_skyboxes_dir)) {
    if(fileUpToDate(oldAssetBakeCache, skyboxDir)) {
      continue;
    } else if(fs::is_directory(skyboxDir)) {
      fs::path exportPath = converterState.bakedAssetDir / "skyboxes" / skyboxDir.path().filename().replace_extension(bakedExtensions.cubeMap);
      printf("Beginning bake of skybox asset: %s\n", skyboxDir.path().string().c_str());
      if(convertCubeMapTexture(skyboxDir, exportPath.string().c_str())) {
        converterState.bakedFilePaths.push_back(skyboxDir);
      } else {
        outputErrorMsg("Failed to bake skybox asset: %s\n", skyboxDir.path().string().c_str());
      }
    }
  }

  if(exists(asset_textures_dir)) {
    for(auto const& textureFileEntry: std::filesystem::directory_iterator(asset_textures_dir)) {
      if(fileUpToDate(oldAssetBakeCache, textureFileEntry)) {
        continue;
      } else if(fs::is_regular_file(textureFileEntry)) {
        fs::path exportPath = converterState.bakedAssetDir / "textures" / textureFileEntry.path().filename().replace_extension(bakedExtensions.texture);
        printf("Beginning bake of texture asset: %s\n", textureFileEntry.path().string().c_str());
        if(convertTexture(textureFileEntry, exportPath.string().c_str())) {
          converterState.bakedFilePaths.push_back(textureFileEntry);
        } else {
          outputErrorMsg("Failed to bake texture asset: %s\n", textureFileEntry.path().string().c_str());
        }
      }
    }
  } else {
    outputErrorMsg("Could not find textures asset directory at: %s", asset_textures_dir.string().c_str());
  }

  if(exists(asset_models_dir)) {
    for(auto const& modelFileEntry: std::filesystem::directory_iterator(asset_models_dir)) {
      if(fileUpToDate(oldAssetBakeCache, modelFileEntry)) {
        continue;
      } else if(fs::is_regular_file(modelFileEntry)) {
        fs::path exportPath = converterState.bakedAssetDir / "models" / modelFileEntry.path().filename().replace_extension(bakedExtensions.model);
        printf("%s\n", modelFileEntry.path().string().c_str());
        if(convertModel(modelFileEntry, exportPath.string().c_str())) {
          converterState.bakedFilePaths.push_back(modelFileEntry);
        } else {
          outputErrorMsg("Failed to bake model asset: %s\n", modelFileEntry.path().string().c_str());
        }
      }
    }
  } else {
    outputErrorMsg("Could not find models asset directory at: %s", asset_models_dir.string().c_str());
  }

  // remember baked item
  u32 convertedFilesCount = (u32)converterState.bakedFilePaths.size();
  for(u32 i = 0; i < convertedFilesCount; i++) {
    const fs::path& recentlyBakedFile = converterState.bakedFilePaths[i];
    AssetBakeCachedItem newlyBakedItem;
    newlyBakedItem.originalFileName = recentlyBakedFile.string();
    newlyBakedItem.originalFileLastModified = lastModifiedTimeStamp(recentlyBakedFile);
    AssetBakeCachedItem::BakedFile bakedFile;
    bakedFile.path = recentlyBakedFile.string();
    bakedFile.name = recentlyBakedFile.filename().string();
    bakedFile.ext = recentlyBakedFile.extension().string();
    newlyBakedItem.bakedFiles.push_back(bakedFile);
    newlyCachedItems.push_back(newlyBakedItem);
  }

  writeOutputData(oldAssetBakeCache, converterState);
  saveCache(oldAssetBakeCache, newlyCachedItems);

  return bakeFailed ? -1 : 0;
}

/* Arguments
 *  - u8** compressedBytes: If the image was not compressed, the returned pointer may be the same as uncompressedBytes
 *              - If it is not the same as uncompressed bytes, it must be manually free'd by the callee.
 * Returns false if error occurred during compression.
 */
bool compressImage(u8* uncompressedBytes, u32 width, u32 height, u32 numChannels, u8** compressedBytes, u32* compressedImageSize, TextureFormat* compressedFormat) {
  u32 imageSize = width * height * numChannels;

  struct LOCAL_FUNCS {
    // NOTE: This is only necessary as a workaround for a bug in the Compressinator lib that swizzles red and blue
    // https://github.com/GPUOpen-Tools/compressonator/issues/244 & https://github.com/GPUOpen-Tools/compressonator/issues/247
    static void swizzleRB(u8* pixelChannels, u32 pixelCount, u32 numChannels) {
      u8 tmp;
      for(u32 i = 0; i < pixelCount; i++) {
        u8* redChannel = &pixelChannels[i * numChannels];
        u8* blueChannel = redChannel + 2;
        tmp = *redChannel;
        *redChannel = *blueChannel;
        *blueChannel = tmp;
      }
    }
  };

  CMP_FORMAT srcFormat = CMP_FORMAT_Unknown;
  CMP_FORMAT dstFormat = CMP_FORMAT_Unknown;
  switch(numChannels) {
    case 1: {
      // TODO: Single channel textures should be able to be compacted for GL_COMPRESSED_R11_EAC
      *compressedFormat = TextureFormat_R8;
      *compressedImageSize = imageSize;
      *compressedBytes = (u8*)malloc(*compressedImageSize);
      memcpy(*compressedBytes, uncompressedBytes, *compressedImageSize);
      *compressedBytes = uncompressedBytes;
      return true;
    }
    case 3: {
      /*
       * TODO: We should *NOT* be using ETC2 format for normals. It is just not the right encoding for the job.
       *      - GLES 3.0 only guarantees ETC1, ETC2, EAC, ASTC.
       *      - My personal device also supports a few ATC formats.
       *      - Determine best format from limited selection.
       */
      CMP_FORMAT normalDesiredFormat = CMP_FORMAT_ETC2_RGB;
      srcFormat = CMP_FORMAT_RGB_888;
      dstFormat = CMP_FORMAT_ETC2_RGB;
      *compressedFormat = TextureFormat_ETC2_RGB;

      // TODO: Compressinator lib workaround. Remove when it is fixed.
      LOCAL_FUNCS::swizzleRB(uncompressedBytes, width * height, numChannels);

      break;
    }
    case 4: {
      srcFormat = CMP_FORMAT_RGBA_8888;
      dstFormat = CMP_FORMAT_ETC2_RGBA;
      *compressedFormat = TextureFormat_ETC2_RGBA;

      // TODO: Compressinator lib workaround. Remove when it is fixed.
      LOCAL_FUNCS::swizzleRB(uncompressedBytes, width * height, numChannels);

      break;
    }
    default: {
      assert_release(false && "Error: Asset baker does not yet support images with 2 or greater than 4 channels.");
    }
  }

  CMP_Texture srcTexture = {0};
  srcTexture.dwSize = sizeof(srcTexture);
  srcTexture.dwWidth = (CMP_DWORD)width;
  srcTexture.dwHeight = (CMP_DWORD)height;
  srcTexture.dwPitch = (CMP_DWORD)(width * numChannels);
  srcTexture.format = srcFormat;
  srcTexture.dwDataSize = (CMP_DWORD)imageSize;
  srcTexture.pData = (CMP_BYTE *)uncompressedBytes;
  srcTexture.pMipSet = nullptr;

  CMP_Texture destTexture = {0};
  destTexture.dwSize = sizeof(destTexture);
  destTexture.dwWidth = srcTexture.dwWidth;
  destTexture.dwHeight = srcTexture.dwHeight;
  destTexture.format = dstFormat;
  destTexture.nBlockHeight = 4;
  destTexture.nBlockWidth = 4;
  destTexture.nBlockDepth = 1;
  *compressedImageSize = CMP_CalculateBufferSize(&destTexture);
  *compressedBytes = (u8*)malloc(*compressedImageSize);
  destTexture.dwDataSize = *compressedImageSize;
  destTexture.pData = *compressedBytes;

  CMP_CompressOptions options = {0};
  options.dwSize = sizeof(options);
  options.fquality = 1.0f; // Quality
  options.dwnumThreads = 0; // Number of threads to use per texture set to auto
  options.SourceFormat = srcTexture.format;
  options.DestFormat = destTexture.format;

  // TODO: Uncomment and get compressinator to generate mipmap levels
//    options.genGPUMipMaps = true;
//    options.miplevels = 3;

  try {
    CMP_ERROR cmp_status = CMP_ConvertTexture(&srcTexture, &destTexture, &options, &CompressionCallback);
    if(cmp_status != CMP_OK) { return false; }
  } catch (const std::exception &ex) {
    outputErrorMsg("Error: %s\n", ex.what());
    return false;
  }

  return true;
}

bool convertModel(const fs::path& inputPath, const char* outputFileName) {
  tinygltf::TinyGLTF loader;
  std::string err;
  std::string warn;
  tinygltf::Model tinyGLTFModel;

  ModelInfo modelInfo = {};
  modelInfo.originalFileName = inputPath.string();

  std::vector<char> modelBytes;
  readFile(inputPath.string().c_str(), modelBytes);

  bool ret = loader.LoadBinaryFromMemory(&tinyGLTFModel, &err, &warn, (const unsigned char*)modelBytes.data(), (const u32)modelBytes.size());

  if (!warn.empty()) {
    printf("Warning: %s\n", warn.c_str());
    return false;
  }
  if (!err.empty()) {
    printf("Error: %s\n", err.c_str());
    return false;
  }
  if (!ret) {
    printf("Failed to parse glTF\n");
    return false;
  }

  struct gltfAttributeMetadata {
    u32 accessorIndex;
    u32 numComponents;
    u32 bufferViewIndex;
    u32 bufferIndex;
    u64 bufferByteOffset;
    u64 bufferByteLength;
  };

  const char* positionIndexKeyString = "POSITION";
  const char* normalIndexKeyString = "NORMAL";
  const char* texture0IndexKeyString = "TEXCOORD_0";

  u32 meshCount = (u32)tinyGLTFModel.meshes.size();
  assert_release(meshCount != 0);
  std::vector<tinygltf::Accessor>* gltfAccessors = &tinyGLTFModel.accessors;
  std::vector<tinygltf::BufferView>* gltfBufferViews = &tinyGLTFModel.bufferViews;

  auto populateAttributeMetadata = [gltfAccessors, gltfBufferViews](const char* keyString, const tinygltf::Primitive& gltfPrimitive) -> gltfAttributeMetadata {
    gltfAttributeMetadata result{};
    result.accessorIndex = gltfPrimitive.attributes.at(keyString);
    result.numComponents = tinygltf::GetNumComponentsInType(gltfAccessors->at(result.accessorIndex).type);
    result.bufferViewIndex = gltfAccessors->at(result.accessorIndex).bufferView;
    result.bufferIndex = gltfBufferViews->at(result.bufferViewIndex).buffer;
    result.bufferByteOffset = gltfBufferViews->at(result.bufferViewIndex).byteOffset;
    result.bufferByteLength = gltfBufferViews->at(result.bufferViewIndex).byteLength;
    return result;
  };

  // TODO: Handle models with more than one mesh
  tinygltf::Mesh gltfMesh = tinyGLTFModel.meshes[0];

  assert_release(!gltfMesh.primitives.empty());
  // TODO: handle meshes that have more than one primitive
  tinygltf::Primitive gltfPrimitive = gltfMesh.primitives[0];
  assert_release(gltfPrimitive.indices > -1); // TODO: Should we deal with models that don't have indices?

  // TODO: Allow variability in attributes beyond POSITION, NORMAL, TEXCOORD_0?
  assert_release(gltfPrimitive.attributes.find(positionIndexKeyString) != gltfPrimitive.attributes.end());
  gltfAttributeMetadata positionAttribute = populateAttributeMetadata(positionIndexKeyString, gltfPrimitive);

  f64* minValues = tinyGLTFModel.accessors[positionAttribute.accessorIndex].minValues.data();
  f64* maxValues = tinyGLTFModel.accessors[positionAttribute.accessorIndex].maxValues.data();

  modelInfo.boundingBoxMin[0] = (f32)minValues[0];
  modelInfo.boundingBoxMin[1] = (f32)minValues[1];
  modelInfo.boundingBoxMin[2] = (f32)minValues[2];
  modelInfo.boundingBoxDiagonal[0] = (f32)(maxValues[0] - minValues[0]);
  modelInfo.boundingBoxDiagonal[1] = (f32)(maxValues[1] - minValues[1]);
  modelInfo.boundingBoxDiagonal[2] = (f32)(maxValues[2] - minValues[2]);

  b32 normalAttributesAvailable = gltfPrimitive.attributes.find(normalIndexKeyString) != gltfPrimitive.attributes.end();
  gltfAttributeMetadata normalAttribute = {0};
  if(normalAttributesAvailable) { // normal attribute data
    normalAttribute = populateAttributeMetadata(normalIndexKeyString, gltfPrimitive);
    assert_release(positionAttribute.bufferIndex == normalAttribute.bufferIndex);
  }

  b32 texture0AttributesAvailable = gltfPrimitive.attributes.find(texture0IndexKeyString) != gltfPrimitive.attributes.end();
  gltfAttributeMetadata texture0Attribute = {0};
  if(texture0AttributesAvailable) { // texture 0 uv coord attribute data
    texture0Attribute = populateAttributeMetadata(texture0IndexKeyString, gltfPrimitive);
    assert_release(positionAttribute.bufferIndex == texture0Attribute.bufferIndex);
  }

  // TODO: Handle vertex attributes that don't share the same buffer?
  u32 vertexAttBufferIndex = positionAttribute.bufferIndex;
  assert_release(tinyGLTFModel.buffers.size() > vertexAttBufferIndex);

  u32 indicesAccessorIndex = gltfPrimitive.indices;
  tinygltf::BufferView indicesGLTFBufferView = gltfBufferViews->at(gltfAccessors->at(indicesAccessorIndex).bufferView);
  u32 indicesGLTFBufferIndex = indicesGLTFBufferView.buffer;
  u64 indicesGLTFBufferByteOffset = indicesGLTFBufferView.byteOffset;
  u64 indicesGLTFBufferByteLength = indicesGLTFBufferView.byteLength;

  u64 minOffset = Min(positionAttribute.bufferByteOffset, Min(texture0Attribute.bufferByteOffset, normalAttribute.bufferByteOffset));
  u8* vertexAttributeData = tinyGLTFModel.buffers[vertexAttBufferIndex].data.data();

  modelInfo.indexCount = (u32)gltfAccessors->at(indicesAccessorIndex).count;
  modelInfo.indexTypeSize = tinygltf::GetComponentSizeInBytes(gltfAccessors->at(indicesAccessorIndex).componentType);

  // TODO: Handle the possibility of the three attributes not being side-by-side in the buffer
  u64 sizeOfAttributeData = positionAttribute.bufferByteLength + normalAttribute.bufferByteLength + texture0Attribute.bufferByteLength;
  assert_release(tinyGLTFModel.buffers[vertexAttBufferIndex].data.size() >= sizeOfAttributeData);
  const u32 positionAttributeIndex = 0;
  const u32 normalAttributeIndex = 1;
  const u32 texture0AttributeIndex = 2;

  u8* positionAttributeData = vertexAttributeData + positionAttribute.bufferByteOffset;
  u8* normalAttributeData = vertexAttributeData + normalAttribute.bufferByteOffset;
  u8* uvAttributeData = vertexAttributeData + texture0Attribute.bufferByteOffset;
  u8* indicesData = tinyGLTFModel.buffers[indicesGLTFBufferIndex].data.data() + indicesGLTFBufferByteOffset;

  s32 normalImageIndex = -1;
  s32 albedoImageIndex = -1;

  s32 gltfMaterialIndex = gltfPrimitive.material;
  if(gltfMaterialIndex >= 0) {
    tinygltf::Material gltfMaterial = tinyGLTFModel.materials[gltfMaterialIndex];
    // TODO: Handle more then just TEXCOORD_0 vertex attribute?
    assert_release(gltfMaterial.normalTexture.texCoord == 0 && gltfMaterial.pbrMetallicRoughness.baseColorTexture.texCoord == 0);

    f64* baseColor = gltfMaterial.pbrMetallicRoughness.baseColorFactor.data();
    modelInfo.baseColor[0] = (f32)baseColor[0];
    modelInfo.baseColor[1] = (f32)baseColor[1];
    modelInfo.baseColor[2] = (f32)baseColor[2];
    modelInfo.baseColor[3] = (f32)baseColor[3];

    // NOTE: gltf.textures.samplers gives info about how to magnify/minify textures and how texture wrapping should work
    s32 normalTextureIndex = gltfMaterial.normalTexture.index;
    if(normalTextureIndex >= 0) {
      normalImageIndex = tinyGLTFModel.textures[normalTextureIndex].source;
    }

    s32 baseColorTextureIndex = gltfMaterial.pbrMetallicRoughness.baseColorTexture.index;
    if(baseColorTextureIndex >= 0) {
      albedoImageIndex = tinyGLTFModel.textures[baseColorTextureIndex].source;
    }
  }

  modelInfo.positionAttributeSize = positionAttribute.bufferByteLength;
  modelInfo.normalAttributeSize = normalAttribute.bufferByteLength;
  modelInfo.uvAttributeSize = texture0Attribute.bufferByteLength;
  modelInfo.indicesSize = indicesGLTFBufferByteLength;

  u8* compressedAlbedo = nullptr;
  if(albedoImageIndex != -1) {
    tinygltf::Image& albedoImage = tinyGLTFModel.images[albedoImageIndex];
    u8* albedoImageData = albedoImage.image.data();
    u64 albedoImageSize = albedoImage.image.size();
    u64 albedoImageWidth = albedoImage.width;
    u64 albedoImageHeight = albedoImage.height;
    u64 albedoImageChannels = albedoImage.component;

    modelInfo.albedoTexWidth = albedoImageWidth;
    modelInfo.albedoTexHeight = albedoImageHeight;

    u32 compressedSize;
    TextureFormat compressedFormat;
    bool success = compressImage(albedoImageData, albedoImageWidth, albedoImageHeight, albedoImageChannels, &compressedAlbedo, &compressedSize, &compressedFormat);

    if(!success) {
      std::printf("Error: Something went wrong with compressing albedo texture for %s\n", inputPath.string().c_str());
    }

    modelInfo.albedoTexSize = compressedSize;
    modelInfo.albedoTexFormat = compressedFormat;
  }

  u8* compressedNormal = nullptr;
  if(normalImageIndex != -1) {
    tinygltf::Image& normalImage = tinyGLTFModel.images[normalImageIndex];
    u64 normalImageWidth = normalImage.width;
    u64 normalImageHeight = normalImage.height;

    u8* normalImageData;
    u64 normalImageSize;
    u64 normalImageChannels = 3;
    if(normalImage.component == 3) {
      normalImageData = normalImage.image.data();
      normalImageSize = normalImage.image.size();
    } else if(normalImage.component == 4) {
      u8* normalImageData_fourComponent = normalImage.image.data();
      normalImageSize = normalImageWidth * normalImageHeight * 3;
      normalImageData = (u8*)malloc(normalImageSize);
      u64 pixelCount = normalImageWidth * normalImageHeight;
      for(u64 i = 0; i < pixelCount; i++) {
        u8* fourComponentPixel = &normalImageData_fourComponent[i * 4];
        u8* threeComponentPixel = &normalImageData[i * 3];
        threeComponentPixel[0] = fourComponentPixel[0];
        threeComponentPixel[1] = fourComponentPixel[1];
        threeComponentPixel[2] = fourComponentPixel[2];
      }
    } else {
      assert_release(false && "Normal map has insufficient number of components.");
    }

    modelInfo.normalTexWidth = normalImageWidth;
    modelInfo.normalTexHeight = normalImageHeight;

    u32 compressedSize;
    TextureFormat compressedFormat;

    // TODO: Enable normals when normal compression is better and the project needs more of a need for normals.
    compressedNormal = normalImageData;
    bool success = compressImage(normalImageData, normalImageWidth, normalImageHeight, normalImageChannels, &compressedNormal, &compressedSize, &compressedFormat);
    if(!success) {
      std::printf("Error: Something went wrong with compressing normal texture for %s\n", inputPath.string().c_str());
    }
    modelInfo.normalTexSize = compressedSize;
    modelInfo.normalTexFormat = compressedFormat;

    if(normalImage.component == 4) { free(normalImageData); }
  }

  AssetFile modelAsset = packModel(&modelInfo,
                                   positionAttributeData,
                                   normalAttributeData,
                                   uvAttributeData,
                                   indicesData,
                                   compressedNormal,
                                   compressedAlbedo);

  saveAssetFile(outputFileName, modelAsset);

  free(compressedNormal);
  free(compressedAlbedo);

  return true;
}

bool convertCubeMapTexture(const fs::path& inputDir, const char* outputFilename) {

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
    outputErrorMsg("Skybox directory (%s) was found empty.", inputDir.string().c_str());
    return false;
  }

  fs::path frontPath = (inputDir / "front").replace_extension(ext);
  fs::path backPath = (inputDir / "back").replace_extension(ext);
  fs::path topPath = (inputDir / "top").replace_extension(ext);
  fs::path bottomPath = (inputDir / "bottom").replace_extension(ext);
  fs::path leftPath = (inputDir / "left").replace_extension(ext);
  fs::path rightPath = (inputDir / "right").replace_extension(ext);

  stbi_uc* frontPixels = stbi_load(frontPath.u8string().c_str(), &frontWidth, &frontHeight, &frontChannels, STBI_rgb);
  stbi_uc* backPixels = stbi_load(backPath.u8string().c_str(), &backWidth, &backHeight, &backChannels, STBI_rgb);
  stbi_uc* topPixels = stbi_load(topPath.u8string().c_str(), &topWidth, &topHeight, &topChannels, STBI_rgb);
  stbi_uc* bottomPixels = stbi_load(bottomPath.u8string().c_str(), &bottomWidth, &bottomHeight, &bottomChannels, STBI_rgb);
  stbi_uc* rightPixels = stbi_load(rightPath.u8string().c_str(), &rightWidth, &rightHeight, &rightChannels, STBI_rgb);
  stbi_uc* leftPixels = stbi_load(leftPath.u8string().c_str(), &leftWidth, &leftHeight, &leftChannels, STBI_rgb);

  if(!frontPixels || !backPixels || !topPixels || !bottomPixels || !leftPixels || !rightPixels) {
    outputErrorMsg("Failed to load CubeMap face file for directory %s\n", inputDir.string().c_str());
    return false;
  }

  if(frontWidth != backWidth || frontWidth != topWidth || frontWidth != bottomWidth || frontWidth != leftWidth || frontWidth != rightWidth ||
      frontHeight != backHeight || frontHeight != topHeight || frontHeight != bottomHeight || frontHeight != leftHeight || frontHeight != rightHeight ||
      frontChannels != backChannels || frontChannels != topChannels || frontChannels != bottomChannels || frontChannels != leftChannels || frontChannels != rightChannels) {
    outputErrorMsg("One or more CubeMap faces do not match in either width, height, or number of channels for directory %s\n", inputDir.string().c_str());
    return false;
  }

  if((frontWidth % 4) != 0 || (frontHeight % 4) != 0) {
    outputErrorMsg("CubeMap face widths and heights must be evenly divisible by 4: %s\n", inputDir.string().c_str());
    return false;
  }

  assert_release(topChannels == 3 && "Number of channels for cube map asset does not match desired");

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

  u8* compressedBytes;
  u32 compressedSize;
  TextureFormat compressedFormat;
  bool success = compressImage(cubeMapPixels_fbtbrl, topWidth, topHeight * 6, topChannels, &compressedBytes, &compressedSize, &compressedFormat);

  if(!success) {
    outputErrorMsg("Error: Something went wrong with compressing %s\n", inputDir.string().c_str());
    return false;
  }

  CubeMapInfo info;
  info.format = compressedFormat;
  info.faceWidth = topWidth;
  info.faceHeight = topHeight;
  info.originalFolder = inputDir.string();
  info.faceSize = compressedSize / 6;
  assets::AssetFile cubeMapAssetFile = assets::packCubeMap(&info, compressedBytes);

  if(compressedBytes != cubeMapPixels_fbtbrl){ free(compressedBytes); }
  free(cubeMapPixels_fbtbrl);

  saveAssetFile(outputFilename, cubeMapAssetFile);

  return true;
}

bool convertTexture(const fs::path& inputPath, const char* outputFilename) {
  int texWidth, texHeight, texChannels;

  stbi_uc* pixels = stbi_load(inputPath.u8string().c_str(), &texWidth, &texHeight, &texChannels, STBI_default);

  if(!pixels) {
    outputErrorMsg("Failed to load texture file %s\n", inputPath.string().c_str());
    return false;
  }

  assert_release(texChannels == 3 || texChannels == 1 && "Texture has an unsupported amount of channels.");

  u8* compressedBytes;
  u32 compressedSize;
  TextureFormat compressedFormat;
  bool success = compressImage(pixels, texWidth, texHeight, texChannels, &compressedBytes, &compressedSize, &compressedFormat);

  if(!success) {
    outputErrorMsg("Error: Something went wrong with compressing %s\n", inputPath.string().c_str());
    return false;
  }

  TextureInfo texInfo;
  texInfo.size = compressedSize;
  texInfo.originalFileName = inputPath.string();
  texInfo.width = texWidth;
  texInfo.height = texHeight;
  texInfo.format = compressedFormat;

  assets::AssetFile newImage = assets::packTexture(&texInfo, compressedBytes);
  saveAssetFile(outputFilename, newImage);

  if(compressedBytes != pixels){ free(compressedBytes); }
  stbi_image_free(pixels);

  return true;
}

f64 lastModifiedTimeStamp(const fs::path &file) {
  auto lastModifiedTimePoint = fs::last_write_time(file);
  f64 lastModified = (f64)(lastModifiedTimePoint.time_since_epoch().count());
  return lastModified;
}

bool CompressionCallback(float fProgress, CMP_DWORD_PTR pUser1, CMP_DWORD_PTR pUser2) {
  printf("\rCompression progress = %3.0f  ", fProgress);
  bool abortCompression = false;
  return abortCompression;
}

bool fileUpToDate(const std::unordered_map<std::string, AssetBakeCachedItem> &cache, const fs::path& file) {
  std::string fileName = file.string();
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

std::size_t fileCountInDir(const fs::path& dirPath) {
  std::size_t fileCount = 0u;
  for(auto const& file: std::filesystem::directory_iterator(dirPath)) {
    if(fs::is_regular_file(file)) {
      ++fileCount;
    }
  }
  return fileCount;
}

std::size_t dirCountInDir(const fs::path& dirPath) {
  std::size_t dirCount = 0u;
  for(auto const& file: std::filesystem::directory_iterator(dirPath)) {
    if(fs::is_directory(file)) {
      ++dirCount;
    }
  }
  return dirCount;
}

void replace(std::string &str, const char *oldTokens, u32 oldTokensCount, char newToken) {
  for(char& c: str) {
    for(u32 i = 0; i < oldTokensCount; i++) {
      if(c == oldTokens[i]) {
        c = newToken;
        break;
      }
    }
  }
}

void replaceBackSlashes(std::string &str) {
  for(char& c: str) {
    if(c == '\\') {
      c = '/';
    }
  }
}

void writeOutputData(const std::unordered_map<std::string, AssetBakeCachedItem> &oldCache,
                     const ConverterState &converterState) {
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

void saveCache(const std::unordered_map<std::string, AssetBakeCachedItem> &oldCache,
               const std::vector<AssetBakeCachedItem> &newBakedItems) {
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
      newCacheBakedFiles.push_back(newCacheBakedFile);
    }
    newCacheItemJson[cacheJsonStrings.bakedFiles] = newCacheBakedFiles;
    bakedFiles.push_back(newCacheItemJson);
  }

  cacheJson[cacheJsonStrings.cacheFiles] = bakedFiles;
  std::string jsonString = cacheJson.dump(1);
  writeFile(assetBakerCacheFileName, jsonString);
}

void loadCache(std::unordered_map<std::string, AssetBakeCachedItem> &assetBakeCache) {
  std::vector<char> fileBytes;

  if(readFile(assetBakerCacheFileName, fileBytes)) {
    fs::path cacheFilePath = fs::absolute(fs::path(assetBakerCacheFileName));
    printf("Found asset baker cache file at: (%s)", cacheFilePath.string().c_str());
  } else {
    printf("Could not find asset baker cache file. (%s)", assetBakerCacheFileName);
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
      cachedItem.bakedFiles.push_back(bakedFile);
    }
    assetBakeCache[cachedItem.originalFileName] = cachedItem;
  }
}