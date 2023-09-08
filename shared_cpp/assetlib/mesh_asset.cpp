#include "mesh_asset.h"

const internal_func char* mapVertexFormatToString[] = {
        "Unknown",
#define VertexFormat(name) #name,
#include "vertex_format.incl"
#undef VertexFormat
};

const internal_func char* MESH_FOURCC = "MESH";

const struct {
  const char* vertexFormat = "vertex_format";
  const char* vertexFormatEnumVal = "vertex_format_enum_val";
  const char* vertexBufferSize = "vertex_buffer_size";
  const char* indexBufferSize = "index_buffer_size";
  const char* indexSize = "index_size";
  const char* originalFile = "original_file";
  const char* bounds = "bound";
  const char* compressionMode = "compression_mode";
  const char* compressionModeEnumVal = "compression_mode_enum_val";
} jsonKeys;

const char* vertexFormatToString(assets::VertexFormat format);
u32 vertexFormatToEnumVal(assets::VertexFormat format);

void assets::readMeshInfo(const AssetFile& assetFile, MeshInfo* meshInfo)
{
	nlohmann::json meshJson = nlohmann::json::parse(assetFile.json);

	meshInfo->vertexBufferSize = meshJson[jsonKeys.vertexBufferSize];
	meshInfo->indexBufferSize = meshJson[jsonKeys.indexBufferSize];
	meshInfo->indexSize = static_cast<u8>(meshJson[jsonKeys.indexSize]);

	meshInfo->originalFile = meshJson[jsonKeys.originalFile];

	std::string compressionString = meshJson[jsonKeys.compressionMode];
  u32 compressionModeEnumVal = meshJson[jsonKeys.compressionModeEnumVal];
	meshInfo->compressionMode = CompressionMode(compressionModeEnumVal);

	std::vector<f32> boundsData;
	boundsData.reserve(7);
	boundsData = meshJson[jsonKeys.bounds].get<std::vector<f32>>();

	meshInfo->bounds.origin[0] = boundsData[0];
	meshInfo->bounds.origin[1] = boundsData[1];
	meshInfo->bounds.origin[2] = boundsData[2];

	meshInfo->bounds.radius = boundsData[3];

	meshInfo->bounds.extents[0] = boundsData[4];
	meshInfo->bounds.extents[1] = boundsData[5];
	meshInfo->bounds.extents[2] = boundsData[6];

	std::string vertexFormat = meshJson[jsonKeys.vertexFormat];
  u32 vertexFormatEnumVal = meshJson[jsonKeys.vertexFormatEnumVal];
	meshInfo->vertexFormat = VertexFormat(vertexFormatEnumVal);
}

void assets::unpackMesh(const MeshInfo& info, const char* srcBuffer, size_t sourceSize, char* dstVertexBuffer, char* dstIndexBuffer)
{
	std::vector<char> decompressedBuffer;
  const u64 decompressedBufferSize = info.vertexBufferSize + info.indexBufferSize;
	decompressedBuffer.resize(decompressedBufferSize);

	LZ4_decompress_safe(srcBuffer, decompressedBuffer.data(), (s32)sourceSize, (s32)decompressedBufferSize);

	//copy vertex buffer
	memcpy(dstVertexBuffer, decompressedBuffer.data(), info.vertexBufferSize);

	//copy index buffer
	memcpy(dstIndexBuffer, decompressedBuffer.data() + info.vertexBufferSize, info.indexBufferSize);
}

assets::AssetFile assets::packMesh(const MeshInfo& meshInfo, char* vertexData, char* indexData)
{
  AssetFile file;
  strncpy(file.type, MESH_FOURCC, 4);
	file.version = ASSET_LIB_VERSION;

	nlohmann::json meshJson;
  meshJson[jsonKeys.vertexFormat] = vertexFormatToString(meshInfo.vertexFormat);
  meshJson[jsonKeys.vertexFormatEnumVal] = vertexFormatToEnumVal(meshInfo.vertexFormat);
  meshJson[jsonKeys.vertexBufferSize] = meshInfo.vertexBufferSize;
  meshJson[jsonKeys.indexBufferSize] = meshInfo.indexBufferSize;
  meshJson[jsonKeys.indexSize] = meshInfo.indexSize;
  meshJson[jsonKeys.originalFile] = meshInfo.originalFile;

	std::vector<float> boundsData;
	boundsData.resize(7);

	boundsData[0] = meshInfo.bounds.origin[0];
	boundsData[1] = meshInfo.bounds.origin[1];
	boundsData[2] = meshInfo.bounds.origin[2];

	boundsData[3] = meshInfo.bounds.radius;

	boundsData[4] = meshInfo.bounds.extents[0];
	boundsData[5] = meshInfo.bounds.extents[1];
	boundsData[6] = meshInfo.bounds.extents[2];

  meshJson[jsonKeys.bounds] = boundsData;

	size_t fullSize = meshInfo.vertexBufferSize + meshInfo.indexBufferSize;

	std::vector<char> mergedBuffer;
	mergedBuffer.resize(fullSize);

	//copy vertex buffer
	memcpy(mergedBuffer.data(), vertexData, meshInfo.vertexBufferSize);

	//copy index buffer
	memcpy(mergedBuffer.data() + meshInfo.vertexBufferSize, indexData, meshInfo.indexBufferSize);

	//compress buffer and copy it into the file struct
	size_t worstCaseCompressionSize = LZ4_compressBound(static_cast<int>(fullSize));
	file.binaryBlob.resize(worstCaseCompressionSize);
	int actualCompressionSize = LZ4_compress_default(mergedBuffer.data(), file.binaryBlob.data(), static_cast<int>(mergedBuffer.size()), static_cast<int>(worstCaseCompressionSize));
	file.binaryBlob.resize(actualCompressionSize);

  meshJson[jsonKeys.compressionMode] = compressionModeToString(CompressionMode::LZ4);
  meshJson[jsonKeys.compressionModeEnumVal] = compressionModeToEnumVal(CompressionMode::LZ4);

	file.json = meshJson.dump();

	return file;
}

assets::MeshBounds assets::calculateBounds(Vertex_PNCV_f32* vertices, size_t vertexCount)
{
	MeshBounds bounds{};

  f32 min[3] = { std::numeric_limits<f32>::max(),std::numeric_limits<f32>::max(),std::numeric_limits<f32>::max() };
  f32 max[3] = { std::numeric_limits<f32>::min(),std::numeric_limits<f32>::min(),std::numeric_limits<f32>::min() };

	for (int i = 0; i < vertexCount; i++) {
		min[0] = std::min(min[0], vertices[i].position[0]);
		min[1] = std::min(min[1], vertices[i].position[1]);
		min[2] = std::min(min[2], vertices[i].position[2]);

		max[0] = std::max(max[0], vertices[i].position[0]);
		max[1] = std::max(max[1], vertices[i].position[1]);
		max[2] = std::max(max[2], vertices[i].position[2]);
	}

	bounds.extents[0] = (max[0] - min[0]) / 2.0f;
	bounds.extents[1] = (max[1] - min[1]) / 2.0f;
	bounds.extents[2] = (max[2] - min[2]) / 2.0f;

	bounds.origin[0] = bounds.extents[0] + min[0];
	bounds.origin[1] = bounds.extents[1] + min[1];
	bounds.origin[2] = bounds.extents[2] + min[2];

	// exact bounding sphere radius
	float radSq = 0;
	for (int i = 0; i < vertexCount; i++) {
		float offset[3];
		offset[0] = vertices[i].position[0] - bounds.origin[0];
		offset[1] = vertices[i].position[1] - bounds.origin[1];
		offset[2] = vertices[i].position[2] - bounds.origin[2];

		float distance = (offset[0] * offset[0]) + (offset[1] * offset[1]) + (offset[2] * offset[2]);
    radSq = std::max(radSq, distance);
	}

	bounds.radius = std::sqrt(radSq);

	return bounds;
}

const char* vertexFormatToString(assets::VertexFormat format) {
  return mapVertexFormatToString[vertexFormatToEnumVal(format)];
}

inline u32 vertexFormatToEnumVal(assets::VertexFormat format) {
  return static_cast<u32>(format);
}