#pragma once

const char *saveFileExt = ".json";

struct EntitySaveFormat {
  u32 modelIndex;
  u32 shaderIndex;
  vec3 posXYZ;
  vec3 scaleXYZ;
  f32 yaw;
  b32 flags;
};

struct PortalSaveFormat {
  u32 destination;
  vec3 normalXYZ;
  vec3 centerXYZ;
  vec2 dimensXY;
};

struct ModelSaveFormat {
  u32 index;
  std::string fileName; // NOTE: currently assumed to be in "src/data/models/"
  vec4 baseColor; // optional, alpha 0 means no value
};

struct ShaderSaveFormat {
  u32 index;
  std::string vertexName;
  std::string fragmentName;
  std::string noiseTextureName; // optional, empty string means no value
};

struct DirectionalLightSaveFormat {
  vec3 color;
  f32 power;
  vec3 dirToSource;
};

struct PositionalLightSaveFormat {
  vec3 color;
  f32 power;
  vec3 pos;
};

struct AmbientLightSaveFormat {
  vec3 color;
  f32 power;
};

struct SceneSaveFormat {
  u32 index;
  std::string title;
  std::string skyboxFileName; // optional, empty string means no value
  std::vector<EntitySaveFormat> entities;
  std::vector<PortalSaveFormat> portals;
  std::vector<DirectionalLightSaveFormat> directionalLights;
  std::vector<PositionalLightSaveFormat> positionalLights;
  AmbientLightSaveFormat ambientLightSaveFormat;
};

struct SaveFormat {
  u32 startingSceneIndex;
  std::vector<SceneSaveFormat> scenes;
  std::vector<ModelSaveFormat> models;
  std::vector<ShaderSaveFormat> shaders;
};

SaveFormat originalWorld() {
  SaveFormat saveFormat;
  std::vector<SceneSaveFormat> &scenes = saveFormat.scenes;
  std::vector<ModelSaveFormat> &models = saveFormat.models;
  std::vector<ShaderSaveFormat> &shaders = saveFormat.shaders;

  saveFormat.startingSceneIndex = 0;

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
      "pos_norm.vert",
      "skybox_rhzup_to_lhyup.frag",
      ""
  });

  // models
  models.reserve(6);
  models.push_back({
      0,
      "models/gate.glb",
      vec4{0, 0, 0, 0}
  });
  models.push_back({
      1,
      "models/tetrahedron.glb",
      vec4{1, .4, .4, 1}
  });
  models.push_back({
      2,
      "models/octahedron.glb",
      vec4{.4, 1, .4, 1}
  });
  models.push_back({
      3,
      "models/icosahedron.glb",
      vec4{.9, .9, .9, 1}
  });
  models.push_back({
      4,
      "models/mrsaturn.glb",
      vec4{.4, .4, 1, 1}
  });
  models.push_back({
      5,
      "models/portal_back.glb",
      vec4{0, 0, 0, 0}
  });

  scenes.reserve(5);

  SceneSaveFormat gateScene;
  gateScene.index = 0;
  gateScene.title = "Gate";
  gateScene.skyboxFileName = "cave";
  gateScene.ambientLightSaveFormat.color = vec3{1, 1, 1};
  gateScene.ambientLightSaveFormat.power = 0.3;
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
    vec3{1, 1, 1},
    0.5,
    vec3{0.69, -0.69, 0.23}
  });
  scenes.push_back(gateScene);

  SceneSaveFormat tetrahedronScene;
  tetrahedronScene.index = 1;
  tetrahedronScene.title = "Tetrahedron";
  tetrahedronScene.skyboxFileName = "yellow_cloud";
  tetrahedronScene.ambientLightSaveFormat.color = vec3{0, 0, 0};
  tetrahedronScene.ambientLightSaveFormat.power = 0;
  tetrahedronScene.entities.reserve(2);
  tetrahedronScene.entities.push_back({
      1,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      3 // EntityType_Wireframe & EntityType_Rotating
  });
  tetrahedronScene.entities.push_back({
      5,
      2,
      vec3{0, -1.75, 1.5},
      vec3{3, 0.5, 3},
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

  SceneSaveFormat octahedronScene;
  octahedronScene.index = 2;
  octahedronScene.title = "Octahedron";
  octahedronScene.skyboxFileName = "interstellar";
  octahedronScene.ambientLightSaveFormat.color = vec3{0, 0, 0};
  octahedronScene.ambientLightSaveFormat.power = 0;
  octahedronScene.entities.reserve(2);
  octahedronScene.entities.push_back({
      2,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      3 // EntityType_Wireframe & EntityType_Rotating
  });
  octahedronScene.entities.push_back({
      5,
      2,
      vec3{1.75, 0, 1.5},
      vec3{3, 0.5, 3},
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

  SceneSaveFormat paperScene;
  paperScene.index = 3;
  paperScene.title = "Paper";
  paperScene.skyboxFileName = "calm_sea";
  paperScene.ambientLightSaveFormat.color = vec3{0, 0, 0};
  paperScene.ambientLightSaveFormat.power = 0;
  paperScene.entities.reserve(2);
  paperScene.entities.push_back({
      4,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      3 // EntityType_Wireframe & EntityType_Rotating
  });
  paperScene.entities.push_back({
      5,
      2,
      vec3{-1.75, 0, 1.5},
      vec3{3, 0.5, 3},
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

  SceneSaveFormat icosahedronScene;
  icosahedronScene.index = 4;
  icosahedronScene.title = "Icosahedron";
  icosahedronScene.skyboxFileName = "polluted_earth";
  icosahedronScene.ambientLightSaveFormat.color = vec3{0, 0, 0};
  icosahedronScene.ambientLightSaveFormat.power = 0;
  icosahedronScene.entities.reserve(2);
  icosahedronScene.entities.push_back({
      3,
      1,
      vec3{0, 0, 1.5},
      vec3{1, 1, 1},
      3.35,
      3 // EntityType_Wireframe & EntityType_Rotating
  });
  icosahedronScene.entities.push_back({
      5,
      2,
      vec3{0, 1.75, 1.5},
      vec3{3, 0.5, 3},
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

  return saveFormat;
}