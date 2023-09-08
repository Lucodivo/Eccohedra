#include "material_asset.h"

const internal_func char* mapTransparencyModeToString[] = {
        "Unknown",
#define TransparencyMode(name) #name,
#include "transparency_mode.incl"
#undef TransparencyMode
};

const internal_func char* MATERIAL_FOURCC = "MATX";

const struct {
  const char* baseEffect = "base_effect";
  const char* textures = "textures";
  const char* customProperties = "custom_properties";
  const char* transparencyMode = "transparency_mode";
  const char* transparencyModeEnumVal = "transparency_mode_enum_val";
} jsonKeys;

const char* transparencyModeToString(assets::TransparencyMode transMode);
u32 transparencyModeToEnumVal(assets::TransparencyMode transMode);

assets::MaterialInfo assets::readMaterialInfo(AssetFile* file)
{
	assets::MaterialInfo info;

	nlohmann::json materialJson = nlohmann::json::parse(file->json);
	info.baseEffect = materialJson[jsonKeys.baseEffect];

	for (auto& [key, value] : materialJson[jsonKeys.textures].items()) {
		info.textures[key] = value;
	}

	for (auto& [key, value] : materialJson[jsonKeys.customProperties].items()) {
		info.customProperties[key] = value;
	}

	auto it = materialJson.find(jsonKeys.transparencyModeEnumVal);
	if (it != materialJson.end()) {
    u32 transparencyModeEnumVal = *it;
    info.transparency = TransparencyMode(transparencyModeEnumVal);
	} else { // default to Opaque
    info.transparency = TransparencyMode::Opaque;
  }

	return info;
}

assets::AssetFile assets::packMaterial(MaterialInfo* info)
{
	nlohmann::json materialJson;
  materialJson[jsonKeys.baseEffect] = info->baseEffect;
  materialJson[jsonKeys.textures] = info->textures;
  materialJson[jsonKeys.customProperties] = info->customProperties;
  materialJson[jsonKeys.transparencyMode] = transparencyModeToString(info->transparency);
  materialJson[jsonKeys.transparencyModeEnumVal] = transparencyModeToEnumVal(info->transparency);

	//core file header
	AssetFile file;
  strncpy(file.type, MATERIAL_FOURCC, 4);
	file.version = ASSET_LIB_VERSION;

	std::string stringified = materialJson.dump();
	file.json = stringified;

	return file;
}

const char* transparencyModeToString(assets::TransparencyMode transMode) {
  return mapTransparencyModeToString[transparencyModeToEnumVal(transMode)];
}

inline u32 transparencyModeToEnumVal(assets::TransparencyMode transMode) {
  return static_cast<u32>(transMode);
}