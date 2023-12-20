#pragma once

#define WORLD_INFO_NO_INDEX -1

enum EntityFlags {
  EntityType_Rotating = 1 << 0,
};

struct Entity {
  u32 modelIndex;
  u32 shaderIndex;
  vec3 posXYZ;
  vec3 scaleXYZ;
  f32 yaw;
  b32 flags;
};

struct PortalInfo {
  u32 destination;
  vec2 normalXY;
  vec3 centerXYZ;
  vec3 dimensXYZ;
  bool oneWay;
  s32 backingModelIndex; // equal to WORLD_INFO_NO_INDEX when no backing model
  s32 backingShaderIndex; // ^^
};

struct ModelInfo {
  u32 index;
  std::string fileName;
  vec4 baseColor; // optional, alpha 0 means no value
};

struct ShaderInfo {
  u32 index;
  std::string vertexName;
  std::string fragmentName;
  std::string noiseTextureName; // optional, empty string means no value
};

struct Light {
  // xyz is color, w is power
  vec4 colorAndPower;
  union {
    vec3 pos;
    vec3 dirToSource;
  };
};

struct SceneInfo {
  u32 index;
  std::string title;
  std::string skyboxFileName;
  std::vector<Entity> entities;
  std::vector<PortalInfo> portals;
  std::vector<Light> directionalLights;
  std::vector<Light> positionalLights;
  vec4 ambientLightColorAndPower;
};

struct WorldInfo {
  u32 startingSceneIndex;
  std::vector<SceneInfo> scenes;
  std::vector<ModelInfo> models;
  std::vector<ShaderInfo> shaders;
};

WorldInfo originalWorld() {
  WorldInfo worldInfo;
  std::vector<SceneInfo> &scenes = worldInfo.scenes;
  std::vector<ModelInfo> &models = worldInfo.models;
  std::vector<ShaderInfo> &shaders = worldInfo.shaders;

  worldInfo.startingSceneIndex = 0;

  // shaders
  shaders.reserve(3);
  shaders.push_back({
      0,
      "gate.vert",
      "gate.frag",
      "tiled_musgrave_texture_1_blur"
  });
  shaders.push_back({
      1,
      "pos_norm.vert",
      "single_color.frag",
      ""
  });
  shaders.push_back({
      2,
      "reflect_skybox.vert",
      "reflect_skybox.frag",
      ""
  });

  // models
  models.reserve(6);
  models.push_back({
      0,
      "gate",
      vec4{0, 0, 0, 0}
  });
  models.push_back({
      1,
      "tetrahedron",
      vec4{1, .4, .4, 1}
  });
  models.push_back({
      2,
      "octahedron",
      vec4{.4, 1, .4, 1}
  });
  models.push_back({
      3,
      "icosahedron",
      vec4{.9, .9, .9, 1}
  });
  models.push_back({
      4,
      "dodecahedron",
      vec4{.5, .5, 1, 1}
  });
  models.push_back({
      5,
      "portal_back",
      vec4{0, 0, 0, 0}
  });

  scenes.reserve(5);

  SceneInfo gateScene;
  gateScene.index = 0;
  gateScene.title = "Gate";
  gateScene.skyboxFileName = "cave";
  gateScene.ambientLightColorAndPower = vec4{1, 1, 1, 0.3f};
  gateScene.entities.reserve(1);
  gateScene.entities.push_back({
    0,
    0,
    vec3{0, 0, 1.5},
    vec3{3, 3, 3},
    0,
    0
  });
  gateScene.portals.reserve(4);
  gateScene.portals.push_back({
    1,
    vec2{0, -1},
    vec3{0, -1.5, 1.5},
    vec3{3, 0, 3},
    false,
    WORLD_INFO_NO_INDEX,
    WORLD_INFO_NO_INDEX
  });
  gateScene.portals.push_back({
      2,
      vec2{1, 0},
      vec3{1.5, 0, 1.5},
      vec3{3, 0, 3},
      false,
      WORLD_INFO_NO_INDEX,
      WORLD_INFO_NO_INDEX
  });
  gateScene.portals.push_back({
      3,
      vec2{-1, 0},
      vec3{-1.5, 0, 1.5},
      vec3{3, 0, 3},
      false,
      WORLD_INFO_NO_INDEX,
      WORLD_INFO_NO_INDEX
  });
  gateScene.portals.push_back({
      4,
      vec2{0, 1},
      vec3{0, 1.5, 1.5},
      vec3{3, 0, 3},
      false,
      WORLD_INFO_NO_INDEX,
      WORLD_INFO_NO_INDEX
  });
  gateScene.directionalLights.reserve(1);
  gateScene.directionalLights.push_back({
    vec4{1, 1, 1, 0.5f},
    vec3{-1.0, -1.0, 1.0}
  });
  scenes.push_back(gateScene);

  SceneInfo tetrahedronScene;
  tetrahedronScene.index = 1;
  tetrahedronScene.title = "Tetrahedron";
  tetrahedronScene.skyboxFileName = "yellow_cloud";
  tetrahedronScene.ambientLightColorAndPower = vec4{1, 1, 1, 0.85};
  tetrahedronScene.entities.reserve(2);
  tetrahedronScene.entities.push_back({
      1,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Rotating
  });
  tetrahedronScene.portals.reserve(1);
  tetrahedronScene.portals.push_back({
      0,
      vec2{0.0, -1.0},
      vec3{-5.0, -1.0, 1.5},
      vec3{3, 0.5f, 3},
      true,
      5,
      2
  });
  tetrahedronScene.directionalLights.push_back({
                                            vec4{1, 1, 1, 0.15f},
                                            vec3{-1.0, -1.0, 1.0}
                                        });
  scenes.push_back(tetrahedronScene);

  SceneInfo octahedronScene;
  octahedronScene.index = 2;
  octahedronScene.title = "Octahedron";
  octahedronScene.skyboxFileName = "interstellar";
  octahedronScene.ambientLightColorAndPower = vec4{1, 1, 1, 0.8f};
  octahedronScene.entities.reserve(2);
  octahedronScene.entities.push_back({
      2,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Rotating
  });
  octahedronScene.portals.reserve(1);
  octahedronScene.portals.push_back({
      0,
      vec2{-1, 0},
      vec3{1.5, 0, 1.5},
      vec3{3, 0.5f, 3},
      false,
      5,
      2
  });
  octahedronScene.directionalLights.push_back({
                                                   vec4{1, 1, 1, 0.2f},
                                                   vec3{-1.0, -1.0, 1.0}
                                               });
  scenes.push_back(octahedronScene);

  SceneInfo dodecahedronScene;
  dodecahedronScene.index = 3;
  dodecahedronScene.title = "Dodecahedron";
  dodecahedronScene.skyboxFileName = "calm_sea";
  dodecahedronScene.ambientLightColorAndPower = vec4{1, 1, 1, 0.7f};
  dodecahedronScene.entities.reserve(2);
  dodecahedronScene.entities.push_back({
      4,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Rotating
  });
  dodecahedronScene.portals.reserve(1);
  dodecahedronScene.portals.push_back({
      0,
      vec2{-SqrtTwoOverTwo32, SqrtTwoOverTwo32},
      vec3{-5.0f, 5.0f, 1.5},
      vec3{3, 0.5f, 3},
      true,
      5,
      2
  });
  dodecahedronScene.directionalLights.push_back({
                                                  vec4{1, 1, 1, 0.3f},
                                                  vec3{-1.0, -1.0, 1.0}
                                              });
  scenes.push_back(dodecahedronScene);

  SceneInfo icosahedronScene;
  icosahedronScene.index = 4;
  icosahedronScene.title = "Icosahedron";
  icosahedronScene.skyboxFileName = "polluted_earth";
  icosahedronScene.ambientLightColorAndPower = vec4{1, 1, 1, 0.7f};
  icosahedronScene.entities.reserve(2);
  icosahedronScene.entities.push_back({
      3,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Rotating
  });
  icosahedronScene.portals.reserve(1);
  icosahedronScene.portals.push_back({
      0,
      vec2{-SqrtTwoOverTwo32, SqrtTwoOverTwo32},
      vec3{5.0, 5.0, 1.5},
      vec3{3, 0.5f, 3},
      true,
      5,
      2
  });
  icosahedronScene.directionalLights.push_back({
                                             vec4{1, 1, 1, 0.3f},
                                             vec3{-1.0, -1.0, 1.0}
                                         });
  scenes.push_back(icosahedronScene);

  return worldInfo;
}