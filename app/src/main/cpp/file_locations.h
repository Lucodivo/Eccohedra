#pragma once

// ==== shaders ====
#define COMMON_SHADER_BASE "shaders/"
const char* posVertexShaderFileLoc = COMMON_SHADER_BASE"pos.vert";
const char* skyboxVertexShaderFileLoc = COMMON_SHADER_BASE"skybox.vert";
const char* singleColorFragmentShaderFileLoc = COMMON_SHADER_BASE"single_color.frag";
const char* skyboxFragmentShaderFileLoc = COMMON_SHADER_BASE"skybox_rhzup_to_lhyup.frag";
const char* gateFragmentShaderFileLoc = COMMON_SHADER_BASE"gat.frag";
const char* blackFragmentShaderFileLoc = COMMON_SHADER_BASE"black.frag";

// Textures
#define COMMON_TEXTURE_BASE "textures/"
const char* tiledDisplacementTextureFileLoc = COMMON_TEXTURE_BASE"tiled_musgrave_texture_1_blur.png";

// Skybox Cube Map textures
#define COMMON_SKYBOX_BASE "skyboxes/"
#define skybox(folder, extension) {  \
COMMON_SKYBOX_BASE#folder"/front."#extension,              \
COMMON_SKYBOX_BASE#folder"/back."#extension,               \
COMMON_SKYBOX_BASE#folder"/top."#extension,                \
COMMON_SKYBOX_BASE#folder"/bottom."#extension,             \
COMMON_SKYBOX_BASE#folder"/right."#extension,              \
COMMON_SKYBOX_BASE#folder"/left."#extension                \
}
#define SKYBOX_TEXTURE_LOCATION_INDEX_FRONT 0
#define SKYBOX_TEXTURE_LOCATION_INDEX_BACK 1
#define SKYBOX_TEXTURE_LOCATION_INDEX_TOP 2
#define SKYBOX_TEXTURE_LOCATION_INDEX_BOTTOM 3
#define SKYBOX_TEXTURE_LOCATION_INDEX_RIGHT 4
#define SKYBOX_TEXTURE_LOCATION_INDEX_LEFT 5
const char* yellowCloudFaceLocations[6] = skybox(yellow_cloud, jpg);
const char* calmSeaFaceLocations[6] = skybox(calm_sea, jpg);
const char* interstellarFaceLocations[6] = skybox(interstellar, png);
const char* pollutedEarthFaceLocations[6] = skybox(polluted_earth, jpg);
const char* caveFaceLocations[6] = skybox(cave, png);

// ==== Scenes ====
#define COMMON_SCENE_BASE "misc/noop_worlds/"
const char* originalSceneLoc = COMMON_SCENE_BASE"original_world.json";