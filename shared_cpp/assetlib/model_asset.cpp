#include "model_asset.h"

const internal_func char* MODEL_FOURCC = "modl";

const struct {
  const char* positionAttributeSize = "positionAttributeSize";
  const char* normalAttributeSize = "normalAttributeSize";
  const char* uvAttributeSize = "uvAttributeSize";
  const char* indicesSize = "indicesSize";
  const char* indexTypeSize = "indexTypeSize";
  const char* indexCount = "indexCount";
  const char* baseColor = "baseColor";
  const char* boundingBoxMin = "boundingBoxMin";
  const char* boundingBoxDiagonal = "boundingBoxDiagonal";
  const char* normalTexFormat = "normalTexFormat";
  const char* normalTexSize = "normalTexSize";
  const char* normalTexWidth = "normalTexWidth";
  const char* normalTexHeight = "normalTexHeight";
  const char* albedoTexFormat = "albedoTexFormat";
  const char* albedoTexSize = "albedoTexSize";
  const char* albedoTexWidth = "albedoTexWidth";
  const char* albedoTexHeight = "albedoTexHeight";
  const char* albedoTexChannels = "albedoTexChannels";
  const char* originalFileName = "originalFileName";
} jsonKeys;

void assets::readModelInfo(const AssetFile& file, ModelInfo* info) {
  nlohmann::json modelJson = nlohmann::json::parse(file.json);

  info->positionAttributeSize = modelJson[jsonKeys.positionAttributeSize];
  info->normalAttributeSize = modelJson[jsonKeys.normalAttributeSize];
  info->uvAttributeSize = modelJson[jsonKeys.uvAttributeSize];
  info->indicesSize = modelJson[jsonKeys.indicesSize];
  info->indexTypeSize = modelJson[jsonKeys.indexTypeSize];
  info->indexCount = modelJson[jsonKeys.indexCount];
  nlohmann::json baseColor = modelJson[jsonKeys.baseColor];
  info->baseColor[0] = baseColor[0];
  info->baseColor[1] = baseColor[1];
  info->baseColor[2] = baseColor[2];
  info->baseColor[3] = baseColor[3];
  nlohmann::json boundingBoxMin = modelJson[jsonKeys.boundingBoxMin];
  info->boundingBoxMin[0] = boundingBoxMin[0];
  info->boundingBoxMin[1] = boundingBoxMin[1];
  info->boundingBoxMin[2] = boundingBoxMin[2];
  nlohmann::json boundingBoxDiagonal = modelJson[jsonKeys.boundingBoxDiagonal];
  info->boundingBoxDiagonal[0] = boundingBoxDiagonal[0];
  info->boundingBoxDiagonal[1] = boundingBoxDiagonal[1];
  info->boundingBoxDiagonal[2] = boundingBoxDiagonal[2];
  info->normalTexFormat = modelJson[jsonKeys.normalTexFormat];
  info->normalTexSize = modelJson[jsonKeys.normalTexSize];
  info->normalTexWidth = modelJson[jsonKeys.normalTexWidth];
  info->normalTexHeight = modelJson[jsonKeys.normalTexHeight];
  info->albedoTexFormat = modelJson[jsonKeys.albedoTexFormat];
  info->albedoTexSize = modelJson[jsonKeys.albedoTexSize];
  info->albedoTexWidth = modelJson[jsonKeys.albedoTexWidth];
  info->albedoTexHeight = modelJson[jsonKeys.albedoTexHeight];
  info->originalFileName = modelJson[jsonKeys.originalFileName];
}

assets::AssetFile assets::packModel(ModelInfo* info,
                                      void* posAttData,
                                      void* normalAttData,
                                      void* uvAttData,
                                      void* indexData,
                                      void* normalTexData,
                                      void* albedoTexData) {

  //core file header
  AssetFile file;
  strncpy(file.type, MODEL_FOURCC, 4);
  file.version = ASSET_LIB_VERSION;

  u64 totalBlobSize = info->positionAttributeSize +
                      info->uvAttributeSize +
                      info->normalAttributeSize +
                      info->indicesSize +
                      info->albedoTexSize +
                      info->normalTexSize;

  nlohmann::json modelJson;
  modelJson[jsonKeys.positionAttributeSize] = info->positionAttributeSize;
  modelJson[jsonKeys.normalAttributeSize] = info->normalAttributeSize;
  modelJson[jsonKeys.uvAttributeSize] = info->uvAttributeSize;
  modelJson[jsonKeys.indicesSize] = info->indicesSize;
  modelJson[jsonKeys.indexTypeSize] = info->indexTypeSize;
  modelJson[jsonKeys.indexCount] = info->indexCount;
  modelJson[jsonKeys.baseColor] = nlohmann::json::array({
    info->baseColor[0],
    info->baseColor[1],
    info->baseColor[2],
    info->baseColor[3]
  });
  modelJson[jsonKeys.boundingBoxMin] = nlohmann::json::array({
    info->boundingBoxMin[0],
    info->boundingBoxMin[1],
    info->boundingBoxMin[2]
  });
  modelJson[jsonKeys.boundingBoxDiagonal] = nlohmann::json::array({
    info->boundingBoxDiagonal[0],
    info->boundingBoxDiagonal[1],
    info->boundingBoxDiagonal[2]
  });
  modelJson[jsonKeys.normalTexFormat] = info->normalTexFormat;
  modelJson[jsonKeys.normalTexSize] = info->normalTexSize;
  modelJson[jsonKeys.normalTexWidth] = info->normalTexWidth;
  modelJson[jsonKeys.normalTexHeight] = info->normalTexHeight;
  modelJson[jsonKeys.albedoTexFormat] = info->albedoTexFormat;
  modelJson[jsonKeys.albedoTexSize] = info->albedoTexSize;
  modelJson[jsonKeys.albedoTexWidth] = info->albedoTexWidth;
  modelJson[jsonKeys.albedoTexHeight] = info->albedoTexHeight;
  modelJson[jsonKeys.originalFileName] = info->originalFileName;
  file.json = modelJson.dump(); // json map to string

  file.binaryBlob.resize(totalBlobSize);
  char* binaryBlobData = file.binaryBlob.data();
  memcpy(binaryBlobData, posAttData, info->positionAttributeSize);
  binaryBlobData += info->positionAttributeSize;
  memcpy(binaryBlobData, normalAttData, info->normalAttributeSize);
  binaryBlobData += info->normalAttributeSize;
  memcpy(binaryBlobData, uvAttData, info->uvAttributeSize);
  binaryBlobData += info->uvAttributeSize;
  memcpy(binaryBlobData, indexData, info->indicesSize);
  binaryBlobData += info->indicesSize;
  memcpy(binaryBlobData, albedoTexData, info->albedoTexSize);
  binaryBlobData += info->albedoTexSize;
  memcpy(binaryBlobData, normalTexData, info->normalTexSize);
  binaryBlobData += info->normalTexSize;

  assert((binaryBlobData - file.binaryBlob.data()) == totalBlobSize);

  return file;
}

assets::ModelDataPtrs assets::ModelInfo::calcDataPts(char* data) {
  ModelDataPtrs modelDataPtrs;
  char* dataTraversalHead = data;
  modelDataPtrs.vertAtts = dataTraversalHead; // Note: Always assumed to be present
  modelDataPtrs.posVertAttOffset = 0;
  modelDataPtrs.normalVertAttOffset = positionAttributeSize;
  modelDataPtrs.uvVertAttOffset = positionAttributeSize + normalAttributeSize;
  dataTraversalHead += positionAttributeSize + normalAttributeSize + uvAttributeSize;
  modelDataPtrs.indices = (indicesSize == 0) ? nullptr : dataTraversalHead;
  dataTraversalHead += indicesSize;
  modelDataPtrs.albedoTex = (albedoTexSize == 0) ? nullptr : dataTraversalHead;
  dataTraversalHead += albedoTexSize;
  modelDataPtrs.normalTex = (normalTexSize == 0) ? nullptr : dataTraversalHead;
  return modelDataPtrs;
}
