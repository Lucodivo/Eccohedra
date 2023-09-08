#pragma once

#include "asset_loader.h"

namespace assets {
	enum class TransparencyMode : u32 {
    Unknown = 0,
#define TransparencyMode(name) name,
#include "transparency_mode.incl"
#undef TransparencyMode
	};

	struct MaterialInfo {
		std::string baseEffect; // info about shader to use (ex: "defaultPBR")
		std::unordered_map<std::string /* name */, std::string /* path */> textures;
		std::unordered_map<std::string, std::string> customProperties;
		TransparencyMode transparency;
	};

	MaterialInfo readMaterialInfo(AssetFile* file);
	AssetFile packMaterial(MaterialInfo* info);
}