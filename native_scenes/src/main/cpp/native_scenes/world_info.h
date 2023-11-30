#pragma once

const f32 portalDepth = 0.5f;
const f32 portalWidth = 3.0f;

enum EntityFlags {
  EntityType_Rotating = 1 << 0,
  EntityType_Wireframe = 1 << 1,
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
  vec3 normalXYZ;
  vec3 centerXYZ;
  vec2 dimensXY;
};

struct ModelInfo {
  u32 index;
  std::string fileName; // NOTE: currently assumed to be in "src/data/models/"
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
  std::string skyboxFileName; // optional, empty string means no value
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
      "pos.vert",
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
      "torus",
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
    vec3{0, -1, 0},
    vec3{0, -1.5, 1.5},
    vec2{3, 3}
  });
  gateScene.portals.push_back({
      2,
      vec3{1, 0, 0},
      vec3{1.5, 0, 1.5},
      vec2{3, 3}
  });
  gateScene.portals.push_back({
      3,
      vec3{-1, 0, 0},
      vec3{-1.5, 0, 1.5},
      vec2{3, 3}
  });
  gateScene.portals.push_back({
      4,
      vec3{0, 1, 0},
      vec3{0, 1.5, 1.5},
      vec2{3, 3}
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
  tetrahedronScene.ambientLightColorAndPower = vec4{0, 0, 0, 0.0f};
  tetrahedronScene.entities.reserve(2);
  tetrahedronScene.entities.push_back({
      1,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Wireframe | EntityType_Rotating
  });
  tetrahedronScene.entities.push_back({
      5,
      2,
      vec3{0, -1.75, 1.5},
      vec3{portalWidth, portalDepth, 3},
      Pi32,
      0
  });
  tetrahedronScene.portals.reserve(1);
  tetrahedronScene.portals.push_back({
      0,
      vec3{0, 1, 0},
      vec3{0, -1.5, 1.5},
      vec2{3, 3}
  });
  scenes.push_back(tetrahedronScene);

  SceneInfo octahedronScene;
  octahedronScene.index = 2;
  octahedronScene.title = "Octahedron";
  octahedronScene.skyboxFileName = "interstellar";
  octahedronScene.ambientLightColorAndPower = vec4{0, 0, 0, 0};
  octahedronScene.entities.reserve(2);
  octahedronScene.entities.push_back({
      2,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Wireframe | EntityType_Rotating
  });
  octahedronScene.entities.push_back({
      5,
      2,
      vec3{1.75, 0, 1.5},
      vec3{portalWidth, portalDepth, 3},
      -(Pi32 * 0.5f),
      0
  });
  octahedronScene.portals.reserve(1);
  octahedronScene.portals.push_back({
      0,
      vec3{-1, 0, 0},
      vec3{1.5, 0, 1.5},
      vec2{3, 3}
  });
  scenes.push_back(octahedronScene);

  SceneInfo paperScene;
  paperScene.index = 3;
  paperScene.title = "Paper";
  paperScene.skyboxFileName = "calm_sea";
  paperScene.ambientLightColorAndPower = vec4{0, 0, 0, 0};
  paperScene.entities.reserve(2);
  paperScene.entities.push_back({
      4,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Wireframe | EntityType_Rotating
  });
  paperScene.entities.push_back({
      5,
      2,
      vec3{-1.75, 0, 1.5},
      vec3{portalWidth, portalDepth, 3},
      Pi32 * 0.5f,
      0
  });
  paperScene.portals.reserve(1);
  paperScene.portals.push_back({
      0,
      vec3{1, 0, 0},
      vec3{-1.5, 0, 1.5},
      vec2{3, 3}
  });
  scenes.push_back(paperScene);

  SceneInfo icosahedronScene;
  icosahedronScene.index = 4;
  icosahedronScene.title = "Icosahedron";
  icosahedronScene.skyboxFileName = "polluted_earth";
  icosahedronScene.ambientLightColorAndPower = vec4{0, 0, 0, 0};
  icosahedronScene.entities.reserve(2);
  icosahedronScene.entities.push_back({
      3,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      EntityType_Wireframe | EntityType_Rotating
  });
  icosahedronScene.entities.push_back({
      5,
      2,
      vec3{0, 1.75, 1.5},
      vec3{portalWidth, portalDepth, 3},
      0,
      0
  });
  icosahedronScene.portals.reserve(1);
  icosahedronScene.portals.push_back({
      0,
      vec3{0, -1, 0},
      vec3{0, 1.5, 1.5},
      vec2{3, 3}
  });
  scenes.push_back(icosahedronScene);

  return worldInfo;
}