#pragma once

const char* saveFileExt = ".json";

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
  std::string skyboxDir; // optional, empty string means no value
  std::string skyboxExt; // optional, empty string means no value
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

void save(const SaveFormat& saveFormat, const char* saveFileName) {
  nlohmann::json saveJson{};

  saveJson["startingSceneIndex"] = saveFormat.startingSceneIndex;

  { // models
    nlohmann::json modelsJson;
    size_t modelCount = saveFormat.models.size();
    for(size_t modelIndex = 0; modelIndex < modelCount; modelIndex++) {
      ModelSaveFormat modelSaveFormat = saveFormat.models[modelIndex];
      modelsJson.push_back({
               {"index", modelSaveFormat.index},
               {"fileName", modelSaveFormat.fileName}
      });
      if(modelSaveFormat.baseColor.a != 0.0f) {
        modelsJson[modelIndex]["baseColor"] = {
                modelSaveFormat.baseColor.r,
                modelSaveFormat.baseColor.g,
                modelSaveFormat.baseColor.b,
                modelSaveFormat.baseColor.a
        };
      }
    }
    saveJson["models"] = modelsJson;
  }

  { // shaders
    nlohmann::json shadersJson;
    size_t shaderCount = saveFormat.shaders.size();
    for(u32 shaderIndex = 0; shaderIndex < shaderCount; shaderIndex++) {
      ShaderSaveFormat shaderSaveFormat = saveFormat.shaders[shaderIndex];
      shadersJson.push_back({
        {"index", shaderSaveFormat.index},
        {"vertexName", shaderSaveFormat.vertexName},
        {"fragmentName", shaderSaveFormat.fragmentName}
      });
      if(!shaderSaveFormat.noiseTextureName.empty()) {
        shadersJson[shaderIndex]["noiseTextureName"] = shaderSaveFormat.noiseTextureName;
      }
    }
    saveJson["shaders"] = shadersJson;
  }

  {
    nlohmann::json scenesJson;
    size_t sceneCount = saveFormat.scenes.size();
    for(u32 sceneIndex = 0; sceneIndex < sceneCount; sceneIndex++) {
      nlohmann::json sceneJson;
      SceneSaveFormat sceneSaveFormat = saveFormat.scenes[sceneIndex];

      sceneJson["index"] = sceneSaveFormat.index;
      if(!sceneSaveFormat.title.empty()) {
        sceneJson["title"] = sceneSaveFormat.title;
      }
      if(!sceneSaveFormat.skyboxDir.empty() && !sceneSaveFormat.skyboxExt.empty()) {
        sceneJson["skyboxDir"] = sceneSaveFormat.skyboxDir;
        sceneJson["skyboxExt"] = sceneSaveFormat.skyboxExt;
      }

      const size_t entityCount = sceneSaveFormat.entities.size();
      for(size_t entityIndex = 0; entityIndex < entityCount; entityIndex++) {
        EntitySaveFormat entitySaveFormat = sceneSaveFormat.entities[entityIndex];
        sceneJson["entities"].push_back({
          {"modelIndex", entitySaveFormat.modelIndex},
          {"shaderIndex", entitySaveFormat.shaderIndex},
          {"posXYZ", {entitySaveFormat.posXYZ.x, entitySaveFormat.posXYZ.y, entitySaveFormat.posXYZ.z}},
          {"scaleXYZ", {entitySaveFormat.scaleXYZ.x, entitySaveFormat.scaleXYZ.y, entitySaveFormat.scaleXYZ.z}},
          {"yaw", entitySaveFormat.yaw},
          {"flags", entitySaveFormat.flags}
        });
      }

      const size_t portalCount = sceneSaveFormat.portals.size();
      for(size_t portalIndex = 0; portalIndex < portalCount; portalIndex++) {
        PortalSaveFormat portalSaveFormat = sceneSaveFormat.portals[portalIndex];
        sceneJson["portals"].push_back({
          {"destination", portalSaveFormat.destination},
          {"centerXYZ", {portalSaveFormat.centerXYZ.x, portalSaveFormat.centerXYZ.y, portalSaveFormat.centerXYZ.z}},
          {"normalXYZ", {portalSaveFormat.normalXYZ.x, portalSaveFormat.normalXYZ.y, portalSaveFormat.normalXYZ.z}},
          {"dimensXY", {portalSaveFormat.dimensXY.x, portalSaveFormat.dimensXY.y}}
        });
      }

      { // lights
        nlohmann::json lightsJson;
        const size_t dirLightCount = sceneSaveFormat.directionalLights.size();
        for(u32 dirLightIndex = 0; dirLightIndex < dirLightCount; dirLightIndex++) {
          DirectionalLightSaveFormat dirLightSaveFormat = sceneSaveFormat.directionalLights[dirLightIndex];
          lightsJson["directional"].push_back({
                  {"color", {dirLightSaveFormat.color.r, dirLightSaveFormat.color.g, dirLightSaveFormat.color.b}},
                  {"power", dirLightSaveFormat.power},
                  {"dirToSource", {dirLightSaveFormat.dirToSource.x, dirLightSaveFormat.dirToSource.y, dirLightSaveFormat.dirToSource.z}}
          });
        }

        const size_t posLightCount = sceneSaveFormat.positionalLights.size();
        for(u32 posLightIndex = 0; posLightIndex < posLightCount; posLightIndex++) {
          PositionalLightSaveFormat posLightSaveFormat = sceneSaveFormat.positionalLights[posLightIndex];
          lightsJson["positional"].push_back({
                  {"color", {posLightSaveFormat.color.r, posLightSaveFormat.color.g, posLightSaveFormat.color.b}},
                  {"power", posLightSaveFormat.power},
                  {"pos", {posLightSaveFormat.pos.x, posLightSaveFormat.pos.y, posLightSaveFormat.pos.z}}
          });
        }

        if(sceneSaveFormat.ambientLightSaveFormat.power != 0) {
          lightsJson["ambient"]["color"] = {
                  sceneSaveFormat.ambientLightSaveFormat.color.r,
                  sceneSaveFormat.ambientLightSaveFormat.color.g,
                  sceneSaveFormat.ambientLightSaveFormat.color.b
          };
          lightsJson["ambient"]["power"] = sceneSaveFormat.ambientLightSaveFormat.power;
        }

        sceneJson["lights"] = lightsJson;
      }

      scenesJson.push_back(sceneJson);
    }
    saveJson["scenes"] = scenesJson;
  }

  // write prettified JSON to another file
  std::ofstream o(saveFileName);
  o << std::setw(4) << saveJson << std::endl;
}

SaveFormat loadSave(const char* saveJson) {
  SaveFormat saveFormat{};

  nlohmann::json json;
  { // parse file
    std::ifstream sceneJsonFileInput(saveJson);
    sceneJsonFileInput >> json;
  }

  size_t sceneCount = json["scenes"].size();
  size_t modelCount = json["models"].size();
  size_t shaderCount = json["shaders"].size();

  saveFormat.startingSceneIndex = json["startingSceneIndex"];
  Assert(saveFormat.startingSceneIndex < sceneCount);

  saveFormat.scenes.reserve(sceneCount);
  saveFormat.models.reserve(modelCount);
  saveFormat.shaders.reserve(shaderCount);

  { // shaders
    for(u32 jsonShaderIndex = 0; jsonShaderIndex < shaderCount; jsonShaderIndex++) {
      nlohmann::json shaderJson = json["shaders"][jsonShaderIndex];
      ShaderSaveFormat shaderSaveFormat;
      shaderSaveFormat.index = shaderJson["index"];
      shaderJson["vertexName"].get_to(shaderSaveFormat.vertexName);
      shaderJson["fragmentName"].get_to(shaderSaveFormat.fragmentName);

      if(!shaderJson["noiseTextureName"].is_null()) {
        shaderJson["noiseTextureName"].get_to(shaderSaveFormat.noiseTextureName);
      }

      saveFormat.shaders.push_back(shaderSaveFormat);
    }
  }

  { // models
    for(u32 jsonModelIndex = 0; jsonModelIndex < modelCount; jsonModelIndex++) {
      nlohmann::json modelJson = json["models"][jsonModelIndex];
      ModelSaveFormat modelSaveFormat;
      modelSaveFormat.index = modelJson["index"];
      modelJson["fileName"].get_to(modelSaveFormat.fileName);

      if(!modelJson["baseColor"].is_null()) {
        Assert(modelJson["baseColor"].size() == 4);
        modelSaveFormat.baseColor = {
                  modelJson["baseColor"][0],
                  modelJson["baseColor"][1],
                  modelJson["baseColor"][2],
                  modelJson["baseColor"][3]
        };
      } else {
        modelSaveFormat.baseColor = {0.0f, 0.0f, 0.0f, 0.0f};
      }

      saveFormat.models.push_back(modelSaveFormat);
    }
  }

  { // scenes
    for(u32 jsonSceneIndex = 0; jsonSceneIndex < sceneCount; jsonSceneIndex++) {
      nlohmann::json sceneJson = json["scenes"][jsonSceneIndex];
      SceneSaveFormat sceneSaveFormat{};
      sceneSaveFormat.index = sceneJson["index"];
      Assert(sceneSaveFormat.index < sceneCount);

      if(!sceneJson["title"].is_null()) {
        sceneJson["title"].get_to(sceneSaveFormat.title);
      } else {
        sceneSaveFormat.title.clear();
      }

      if(!sceneJson["skyboxDir"].is_null() && !sceneJson["skyboxExt"].is_null()) { // if we have a skybox...
        sceneJson["skyboxDir"].get_to(sceneSaveFormat.skyboxDir);
        sceneJson["skyboxExt"].get_to(sceneSaveFormat.skyboxExt);
      } else {
        sceneSaveFormat.skyboxDir.clear();
        sceneSaveFormat.skyboxExt.clear();
      }


      size_t entityCount = sceneJson["entities"].size();
      for(u32 jsonEntityIndex = 0; jsonEntityIndex < entityCount; jsonEntityIndex++) {
        nlohmann::json entityJson = sceneJson["entities"][jsonEntityIndex];
        EntitySaveFormat entitySaveFormat;
        entitySaveFormat.modelIndex = entityJson["modelIndex"];
        entitySaveFormat.shaderIndex = entityJson["shaderIndex"];
        Assert(entitySaveFormat.shaderIndex < shaderCount);
        Assert(entityJson["posXYZ"].size() == 3);
        Assert(entityJson["scaleXYZ"].size() == 3);
        entitySaveFormat.posXYZ = {
                entityJson["posXYZ"][0],
                entityJson["posXYZ"][1],
                entityJson["posXYZ"][2]
        };
        entitySaveFormat.scaleXYZ = {
                entityJson["scaleXYZ"][0],
                entityJson["scaleXYZ"][1],
                entityJson["scaleXYZ"][2]
        };
        entitySaveFormat.yaw = entityJson["yaw"];
        entitySaveFormat.flags = entityJson["flags"];
        sceneSaveFormat.entities.push_back(entitySaveFormat);
      }

      size_t portalCount = sceneJson["portals"].size();
      for (u32 jsonPortalIndex = 0; jsonPortalIndex < portalCount; jsonPortalIndex++)
      {
        nlohmann::json portalJson = sceneJson["portals"][jsonPortalIndex];
        PortalSaveFormat portalSaveFormat;
        portalSaveFormat.destination = portalJson["destination"];
        Assert(portalJson["normalXYZ"].size() == 3);
        Assert(portalJson["centerXYZ"].size() == 3);
        Assert(portalJson["dimensXY"].size() == 2);
        portalSaveFormat.normalXYZ = {
                portalJson["normalXYZ"][0],
                portalJson["normalXYZ"][1],
                portalJson["normalXYZ"][2]
        };
        portalSaveFormat.centerXYZ = {
                portalJson["centerXYZ"][0],
                portalJson["centerXYZ"][1],
                portalJson["centerXYZ"][2]
        };
        portalSaveFormat.dimensXY = {
                portalJson["dimensXY"][0],
                portalJson["dimensXY"][1]
        };
        sceneSaveFormat.portals.push_back(portalSaveFormat);
      }

      if(!sceneJson["lights"].is_null()) {
        nlohmann::json lightsJson = sceneJson["lights"];
        if(!lightsJson["directional"].is_null())  {
          size_t directionalLightCount = lightsJson["directional"].size();
          for(u32 jsonDirLightIndex = 0; jsonDirLightIndex < directionalLightCount; jsonDirLightIndex++) {
            nlohmann::json dirLightJson = lightsJson["directional"][jsonDirLightIndex];
            DirectionalLightSaveFormat directionalLightSaveFormat;
            directionalLightSaveFormat.color = {
                    dirLightJson["color"][0],
                    dirLightJson["color"][1],
                    dirLightJson["color"][2]
            };
            directionalLightSaveFormat.power = dirLightJson["power"];
            directionalLightSaveFormat.dirToSource = {
                    dirLightJson["dirToSource"][0],
                    dirLightJson["dirToSource"][1],
                    dirLightJson["dirToSource"][2]
            };
            sceneSaveFormat.directionalLights.push_back(directionalLightSaveFormat);
          }
        }

        if(!lightsJson["positional"].is_null())  {
          size_t positionalLightCount = lightsJson["positional"].size();
          for(u32 jsonPosLightIndex = 0; jsonPosLightIndex < positionalLightCount; jsonPosLightIndex++) {
            nlohmann::json posLightJson = lightsJson["positional"][jsonPosLightIndex];
            PositionalLightSaveFormat positionalLightSaveFormat;
            positionalLightSaveFormat.color = {
                    posLightJson["color"][0],
                    posLightJson["color"][1],
                    posLightJson["color"][2]
            };
            positionalLightSaveFormat.power = posLightJson["power"];
            positionalLightSaveFormat.pos = {
                    posLightJson["pos"][0],
                    posLightJson["pos"][1],
                    posLightJson["pos"][2]
            };
            sceneSaveFormat.positionalLights.push_back(positionalLightSaveFormat);
          }
        }

        if(!lightsJson["ambient"].is_null()) {
          nlohmann::json ambientLightJson = lightsJson["ambient"];
          sceneSaveFormat.ambientLightSaveFormat.color = {
                  ambientLightJson["color"][0],
                  ambientLightJson["color"][1],
                  ambientLightJson["color"][2]
          };
          sceneSaveFormat.ambientLightSaveFormat.power = ambientLightJson["power"];
        }
      }

      saveFormat.scenes.push_back(sceneSaveFormat);
    }
  }

  return saveFormat;
}