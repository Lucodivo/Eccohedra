#include "prefab_asset.h"

const internal_func char* PREFAB_FOURCC = "PRFB";

const struct {
  const char* nodeMatrices = "nodeMatrices";
  const char* nodeNames = "nodeNames";
  const char* nodeParents = "nodeParents";
  const char* nodeMeshes = "nodeMeshes";
  const char* meshPath = "meshPath";
  const char* materialPath = "materialPath";
  const char* compressed_size = "compressed_size";
} jsonKeys;

assets::PrefabInfo assets::readPrefabInfo(AssetFile* file)
{
	PrefabInfo info;
	nlohmann::json prefabMetadata = nlohmann::json::parse(file->json);

  std::unordered_map<u64, s32> nodeMatrices = prefabMetadata[jsonKeys.nodeMatrices];
	for (auto& [key, value] : prefabMetadata[jsonKeys.nodeMatrices].items())
	{
		info.nodeMatrices[value[0]] = value[1];
	}

	for (auto& [key, value]  : prefabMetadata[jsonKeys.nodeNames].items())
	{
		info.nodeNames[value[0]] = value[1];
	}

	for (auto& [key, value] : prefabMetadata[jsonKeys.nodeParents].items())
	{
		info.nodeParents[value[0]] = value[1];
	}

	std::unordered_map<uint64_t, nlohmann::json> meshNodes = prefabMetadata[jsonKeys.nodeMeshes];
	for (auto& [meshIndex, meshJson] : meshNodes) {
		assets::PrefabInfo::NodeMesh node;

		node.meshPath = meshJson[jsonKeys.meshPath];
		node.materialPath = meshJson[jsonKeys.materialPath];

		info.nodeMeshes[meshIndex] = node;
	}

	size_t matrixCount = file->binaryBlob.size() / (sizeof(f32) * 16);
	info.matrices.resize(matrixCount);

	memcpy(info.matrices.data(),file->binaryBlob.data(), file->binaryBlob.size());

	return info;
}

assets::AssetFile assets::packPrefab(const PrefabInfo& info)
{
	nlohmann::json prefabJson;
  prefabJson[jsonKeys.nodeMatrices] = info.nodeMatrices;
  prefabJson[jsonKeys.nodeNames] = info.nodeNames;
  prefabJson[jsonKeys.nodeParents] = info.nodeParents;

	std::unordered_map<uint64_t, nlohmann::json> meshIndex;
	for (auto& [key, value] : info.nodeMeshes)
	{
		nlohmann::json meshNode;
    meshNode[jsonKeys.meshPath] = value.meshPath;
    meshNode[jsonKeys.materialPath] = value.materialPath;
    meshIndex[key] = meshNode;
	}

  prefabJson[jsonKeys.nodeMeshes] = meshIndex;

	//core file header
	AssetFile file;
  strncpy(file.type, PREFAB_FOURCC, 4);
	file.version = 1;

	file.binaryBlob.resize(info.matrices.size() * sizeof(float) * 16);
	memcpy(file.binaryBlob.data(), info.matrices.data(), info.matrices.size() * sizeof(float) * 16);

	std::string stringified = prefabJson.dump();
	file.json = stringified;

	return file;
}
