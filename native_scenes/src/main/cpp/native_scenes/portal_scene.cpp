#define PORTAL_BACKING_BOX_DEPTH 0.5f
#define MAX_PORTALS 4

struct Player {
  BoundingBox boundingBox;
};

enum EntityType {
  EntityType_Rotating = 1 << 0,
  EntityType_Wireframe = 1 << 1
};

struct Entity {
  BoundingBox boundingBox;
  u32 modelIndex;
  u32 shaderIndex;
  b32 typeFlags; // EntityType flags
  vec3 position;
  vec3 scale;
  f32 yaw; // NOTE: Radians. 0 rads starts at {0, -1} and goes around the xy-plane in a CCW as seen from above
};

enum PortalState {
  PortalState_FacingCamera = 1 << 0,
  PortalState_InFocus = 1 << 1
};

struct Portal {
  vec3 normal;
  vec3 centerPosition;
  vec2 dimens;
  b32 stateFlags; // PortalState flags
  u32 stencilMask;
  u32 sceneDestination;
};

struct Light {
  vec4 color;
  // NOTE: For directional lights, this is the direction to the source
  vec3 pos;
};

struct Scene {
  // TODO: should the scene keep track of its own index in the worlds?
  Entity entities[16];
  u32 entityCount;
  Portal portals[MAX_PORTALS];
  u32 portalCount;
  Light dirPosLightStack[8];
  u32 dirLightCount;
  u32 posLightCount;
  vec4 ambientLight;
  GLuint skyboxTexture;
  const char* title;
  const char* skyboxFileName;
};

struct World
{
  Camera camera;
  Player player;
  u32 currentSceneIndex;
  StopWatch stopWatch;
  Scene scenes[16];
  u32 sceneCount;
  Model models[128];
  u32 modelCount;
  f32 fov;
  f32 aspect;
  struct {
    ProjectionViewModelUBO projectionViewModelUbo;
    GLuint projectionViewModelUboId;
    FragUBO fragUbo;
    GLuint fragUboId;
    LightUBO lightUbo;
    GLuint lightUboId;
  } UBOs;
  ShaderProgram shaders[16];
  u32 shaderCount;
} globalWorld{};

const vec3 defaultPlayerDimensionInMeters{0.5f, 0.25f, 1.75f}; // NOTE: ~1'7"w, 9"d, 6'h
const f32 near = 0.1f;
const f32 far = 200.0f;

// TODO: Why is this global?
global_variable GLuint portalQueryObjects_GLOBAL[MAX_PORTALS];

global_variable union {
  struct {
    ShaderProgram singleColor;
    ShaderProgram skybox;
    ShaderProgram stencil;
  };
  ShaderProgram shaders[3];
} shaders_GLOBAL;

void drawScene(World* world, const u32 sceneIndex, u32 stencilMask = 0x00);
void drawPortals(World* world, const u32 sceneIndex);

void addPortal(World* world, u32 homeSceneIndex,
               const vec3& centerPosition, const vec3& normal, const vec2& dimens,
               const u32 stencilMask, const u32 destinationSceneIndex) {
  assert(stencilMask <= MAX_STENCIL_VALUE && stencilMask > 0);

  Scene* homeScene = world->scenes + homeSceneIndex;
  assert(ArrayCount(homeScene->portals) > homeScene->portalCount);

  Portal portal{};
  portal.stencilMask = stencilMask;
  portal.dimens = dimens;
  portal.centerPosition = centerPosition;
  portal.normal = normal;
  portal.sceneDestination = destinationSceneIndex;
  portal.stateFlags = 0;

  homeScene->portals[homeScene->portalCount++] = portal;
}

u32 addNewScene(World* world, const char* title) {
  assert(ArrayCount(world->scenes) > world->sceneCount);
  u32 sceneIndex = world->sceneCount++;
  Scene* scene = world->scenes + sceneIndex;
  *scene = {};
  scene->title = title;
  return sceneIndex;
}

u32 addNewShader(World* world, const char* vertexShaderFileLoc, const char* fragmentShaderFileLoc, const char* noiseTexture = nullptr) {
  assert(ArrayCount(world->shaders) > world->shaderCount);
  u32 shaderIndex = world->shaderCount++;
  ShaderProgram* shader = world->shaders + shaderIndex;
  *shader = createShaderProgram(vertexShaderFileLoc, fragmentShaderFileLoc, noiseTexture);
  return shaderIndex;
}

u32 addNewEntity(World* world, u32 sceneIndex, u32 modelIndex,
                 vec3 pos, vec3 scale, f32 yaw,
                 u32 shaderIndex, b32 entityTypeFlags = 0) {
  Scene* scene = world->scenes + sceneIndex;
  assert(ArrayCount(scene->entities) > scene->entityCount);
  u32 sceneEntityIndex = scene->entityCount++;
  Entity* entity = scene->entities + sceneEntityIndex;
  *entity = {};
  entity->modelIndex = modelIndex;
  entity->boundingBox = world->models[modelIndex].boundingBox;
  entity->boundingBox.min = hadamard(entity->boundingBox.min, scale);
  entity->boundingBox.min += pos;
  entity->boundingBox.diagonal = hadamard(entity->boundingBox.diagonal, scale);
  entity->position = pos;
  entity->scale = scale;
  entity->yaw = yaw;
  entity->shaderIndex = shaderIndex;
  entity->typeFlags = entityTypeFlags;
  return sceneEntityIndex;
}

u32 addNewDirectionalLight(World* world, u32 sceneIndex, vec3 lightColor, f32 lightPower, vec3 lightToSource) {
  Scene* scene = world->scenes + sceneIndex;
  assert(scene->posLightCount + scene->dirLightCount < ArrayCount(scene->dirPosLightStack));
  u32 newLightIndex = scene->dirLightCount++;
  scene->dirPosLightStack[newLightIndex].color.rgb = lightColor;
  scene->dirPosLightStack[newLightIndex].color.a = lightPower;
  scene->dirPosLightStack[newLightIndex].pos = normalize(lightToSource);
  return newLightIndex;
}

u32 addNewPositionalLight(World* world, u32 sceneIndex, vec3 lightColor, f32 lightPower, vec3 lightPos) {
  Scene* scene = world->scenes + sceneIndex;
  const u32 maxLights = ArrayCount(scene->dirPosLightStack);
  assert(scene->posLightCount + scene->dirLightCount < maxLights);
  u32 newLightIndex = maxLights - 1 - scene->posLightCount++;
  scene->dirPosLightStack[newLightIndex].color.rgb = lightColor;
  scene->dirPosLightStack[newLightIndex].color.a = lightPower;
  scene->dirPosLightStack[newLightIndex].pos = lightPos;
  return newLightIndex;
}

void adjustAmbientLight(World* world, u32 sceneIndex, vec3 lightColor, f32 lightPower) {
  Scene* scene = world->scenes + sceneIndex;
  scene->ambientLight.rgb = lightColor;
  scene->ambientLight.a = lightPower;
}

u32 addNewModel(World* world, const char* modelFileLoc) {
  u32 modelIndex = world->modelCount++;
  loadModelAsset(modelFileLoc, world->models + modelIndex);
  return modelIndex;
}

inline vec3 calcBoundingBoxCenterPosition(BoundingBox box) {
  return box.min + (box.diagonal * 0.5f);
}

// assume "eyes" (FPS camera) is positioned on middle X, max Y, max Z
inline vec3 calcPlayerViewingPosition(const Player* player) {
  return player->boundingBox.min + hadamard(player->boundingBox.diagonal, {0.5f, 1.0f, 1.0f});
}

void drawTrianglesWireframe(const VertexAtt* vertexAtt) {
  // TODO: This will not work at all in a general case
  // TODO: It only currently works because the wireframe shapes are the first things we draw in each scene
  // TODO: can't just disable the depth test whenever
  glUseProgram(shaders_GLOBAL.singleColor.id);
  setUniform(shaders_GLOBAL.singleColor.id, "baseColor", vec3{0.0f, 0.0f, 0.0f});
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  drawLines(vertexAtt); // TODO: This probably does not work and should be monitored
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

mat4 calcBoxStencilModelMatFromPortalModelMat(const mat4& portalModelMat) {
  return portalModelMat * scale_mat4(vec3{1.0f, PORTAL_BACKING_BOX_DEPTH, 1.0f}) * translate_mat4(-cubeFaceNegativeYCenter);
}

void drawPortal(const World* world, Portal* portal) {
  glUseProgram(shaders_GLOBAL.stencil.id);

  // NOTE: Stencil function Example
  // GL_LEQUAL
  // Passes if ( ref & mask ) <= ( stencil & mask )
  glStencilFunc(GL_EQUAL, // func
                0xFF, // ref
                0x00); // mask // Only draw portals where the stencil is cleared
  glStencilOp(GL_KEEP, // action when stencil fails
              GL_KEEP, // action when stencil passes but depth fails
              GL_REPLACE); // action when both stencil and depth pass

  mat4 portalModelMat = quadModelMatrix(portal->centerPosition, portal->normal, portal->dimens.x, portal->dimens.y);
  VertexAtt* portalVertexAtt = quadPosVertexAttBuffers(false);
  if(flagIsSet(portal->stateFlags, PortalState_InFocus)) {
    portalModelMat = calcBoxStencilModelMatFromPortalModelMat(portalModelMat);
    portalVertexAtt = cubePosVertexAttBuffers(true, true);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &portalModelMat);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glUseProgram(shaders_GLOBAL.stencil.id);
  glStencilMask(portal->stencilMask);
  drawTriangles(portalVertexAtt);
}

void drawPortals(World* world, const u32 sceneIndex){

  Scene* scene = world->scenes + sceneIndex;

  for(u32 portalIndex = 0; portalIndex < scene->portalCount; portalIndex++) {
    Portal* portal = scene->portals + portalIndex;
    // don't draw portals if portal isn't visible
    // TODO: better visibility tests besides facing camera?
    if(!flagIsSet(portal->stateFlags, PortalState_FacingCamera)) { continue; }

    // begin occlusion query
    glBeginQuery(GL_ANY_SAMPLES_PASSED, portalQueryObjects_GLOBAL[portalIndex]);
    drawPortal(world, portal);
    // end occlusion query
    glEndQuery(GL_ANY_SAMPLES_PASSED);
  }

  // turn off writes to the stencil
  glStencilMask(0x00);

  // Draw portal worlds
  // We need to clear disable depth values so distant objects through the "portals" still get drawn
  // The portals themselves will still obey the depth of the scene, as the stencils have been rendered with depth in mind
  glClear(GL_DEPTH_BUFFER_BIT);

  for(u32 portalIndex = 0; portalIndex < scene->portalCount; portalIndex++) {
    Portal portal = scene->portals[portalIndex];
    // don't draw scene if portal isn't visible
    // TODO: We need to develop and test an algorithm thar more accurately tells us whether a particular portal is in sight of the camera
    if(!flagIsSet(portal.stateFlags, PortalState_FacingCamera)) { continue; }

    vec3 portalNormal_viewSpace = (world->UBOs.projectionViewModelUbo.view * Vec4(-portal.normal, 0.0f)).xyz;
    vec3 portalCenterPos_viewSpace = (world->UBOs.projectionViewModelUbo.view * Vec4(portal.centerPosition, 1.0f)).xyz;
    mat4 portalProjectionMat = obliquePerspective(world->fov, world->aspect, near, far, portalNormal_viewSpace, portalCenterPos_viewSpace);

    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, projection), sizeof(mat4), &portalProjectionMat);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Conditional render only if the any samples passed while drawing the portal
    drawScene(world, portal.sceneDestination, portal.stencilMask);
  }
}

void drawScene(World* world, const u32 sceneIndex, u32 stencilMask) {
  glStencilFunc(
          GL_EQUAL, // test function applied to stored stencil value and ref [ex: discard when stored value GL_GREATER ref]
          stencilMask, // ref
          0xFF); // enable which bits in reference and stored value are compared

  Scene* scene = world->scenes + sceneIndex;

  if(scene->skyboxTexture != TEXTURE_ID_NO_TEXTURE) { // draw skybox if one exists
    glUseProgram(shaders_GLOBAL.skybox.id);
    bindActiveTextureCubeMap(skyboxActiveTextureIndex, scene->skyboxTexture);
    setSamplerCube(shaders_GLOBAL.skybox.id, skyboxTexUniformName, skyboxActiveTextureIndex);
    mat4 identityMat4 = identity_mat4();
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &identityMat4);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    drawTriangles(cubePosVertexAttBuffers(true));
  }

  // update scene light uniform buffer object
  {
    const u32 maxLights = ArrayCount(world->UBOs.lightUbo.dirPosLightStack);
    assert((scene->dirLightCount + scene->posLightCount) <= maxLights);

    // NOTE: The lights are on a single double ended array where directional lights are added to the beginning
    // and directional lights are added to the end.
    // TODO: If LightUniform and Light struct for class were the same we could do a simple memcpy
    world->UBOs.lightUbo.dirLightCount = scene->dirLightCount;
    for(u32 i = 0; i < scene->dirLightCount; ++i) {
      world->UBOs.lightUbo.dirPosLightStack[i].color = scene->dirPosLightStack[i].color;
      world->UBOs.lightUbo.dirPosLightStack[i].pos.xyz = scene->dirPosLightStack[i].pos;
      // TODO: Z component of pos currently undefined and potentially dangerous. Determine if it can be used.
    }

    world->UBOs.lightUbo.posLightCount = scene->posLightCount;
    for(u32 i = 0; i < scene->posLightCount; ++i) {
      world->UBOs.lightUbo.dirPosLightStack[maxLights - i].color = scene->dirPosLightStack[maxLights - i].color;
      world->UBOs.lightUbo.dirPosLightStack[maxLights - i].pos.xyz = scene->dirPosLightStack[maxLights - i].pos;
      // TODO: Z component of pos currently undefined and potentially dangerous. Determine if it can be used.
    }

    world->UBOs.lightUbo.ambientLight = scene->ambientLight;

    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.lightUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightUBO), &world->UBOs.lightUbo);
  }

  for(u32 sceneEntityIndex = 0; sceneEntityIndex < scene->entityCount; ++sceneEntityIndex) {
    Entity* entity = &scene->entities[sceneEntityIndex];
    ShaderProgram shader = world->shaders[entity->shaderIndex];

    mat4 modelMatrix = scaleRotTrans_mat4(entity->scale, vec3{0.0f, 0.0f, 1.0f}, entity->yaw, entity->position);

    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &modelMatrix);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUseProgram(shader.id);
    if(shader.noiseTextureId != TEXTURE_ID_NO_TEXTURE) {
      bindActiveTextureSampler2d(noiseActiveTextureIndex, shader.noiseTextureId);
      setSampler2D(shader.id, noiseTexUniformName, noiseActiveTextureIndex);
    }
    Model model = world->models[entity->modelIndex];
    // TODO: Should some of this logic be moved to drawModel()?
    for(u32 meshIndex = 0; meshIndex < model.meshCount; ++meshIndex) {
      Mesh* mesh = model.meshes + meshIndex;
      if(mesh->textureData.baseColor.a != 0.0f) {
        setUniform(shader.id, baseColorUniformName, mesh->textureData.baseColor.rgb);
      }
      if(mesh->textureData.albedoTextureId != TEXTURE_ID_NO_TEXTURE) {
        bindActiveTextureSampler2d(albedoActiveTextureIndex, mesh->textureData.albedoTextureId);
        setSampler2D(shader.id, albedoTexUniformName, albedoActiveTextureIndex);
      }
      if(mesh->textureData.normalTextureId != TEXTURE_ID_NO_TEXTURE) {
        bindActiveTextureSampler2d(normalActiveTextureIndex, mesh->textureData.normalTextureId);
        setSampler2D(shader.id, normalTexUniformName, normalActiveTextureIndex);
      }

      drawTriangles(&mesh->vertexAtt);
    }

    if(entity->typeFlags & EntityType_Wireframe) { // wireframes should be drawn on top default mesh
      for(u32 meshIndex = 0; meshIndex < model.meshCount; ++meshIndex) {
        Mesh* mesh = model.meshes + meshIndex;
        drawTrianglesWireframe(&mesh->vertexAtt);
      }
    }
  }
}

void drawSceneWithPortals(World* world)
{
  // draw scene
  drawScene(world, world->currentSceneIndex);
  // draw portals
  drawPortals(world, world->currentSceneIndex);
}

void updateEntities(World* world) {
  for(u32 sceneIndex = 0; sceneIndex < world->sceneCount; ++sceneIndex) {
    Scene* scene = world->scenes + sceneIndex;
    for(u32 entityIndex = 0; entityIndex < scene->entityCount; ++entityIndex) {
      Entity* entity = scene->entities + entityIndex;
      if(entity->typeFlags & EntityType_Rotating) {
        entity->yaw += 30.0f * RadiansPerDegree * world->stopWatch.lapInSeconds;
        if(entity->yaw > Tau32) {
          entity->yaw -= Tau32;
        }
      }
    }
  }

  Scene* currentScene = &world->scenes[world->currentSceneIndex];
  vec3 playerViewPosition = calcPlayerViewingPosition(&world->player);
  b32 portalEntered = false;
  u32 portalSceneDestination;
  auto updatePortalsForScene = [playerViewPosition, &portalEntered, &portalSceneDestination](Scene* scene) {
    for(u32 portalIndex = 0; portalIndex < scene->portalCount; ++portalIndex) {
      Portal* portal = scene->portals + portalIndex;

      vec3 portalCenterToPlayerView = playerViewPosition - portal->centerPosition;
      b32 portalFacingCamera = similarDirection(portal->normal, playerViewPosition - portal->centerPosition) ? PortalState_FacingCamera : false;

      b32 portalWasInFocus = flagIsSet(portal->stateFlags, PortalState_InFocus);
      vec3 viewPositionPerpendicularToPortal = perpendicularTo(portalCenterToPlayerView, portal->normal);
      f32 widthDistFromCenter = magnitude(viewPositionPerpendicularToPortal.xy);
      f32 heightDistFromCenter = viewPositionPerpendicularToPortal.z;
      b32 viewerInsideDimens = widthDistFromCenter < (portal->dimens.x * 0.5f) && heightDistFromCenter < (portal->dimens.y * 0.5f);

      b32 portalInFocus = (portalFacingCamera && viewerInsideDimens) ? PortalState_InFocus : false;
      b32 insidePortal = portalWasInFocus && !portalFacingCamera; // portal was in focus and now we're on the other side

      overrideFlags(&portal->stateFlags, portalFacingCamera | portalInFocus);

      if(insidePortal){
        portalEntered = true;
        portalSceneDestination = portal->sceneDestination;
        break;
      }
    }
  };

  // update portals for current scene
  updatePortalsForScene(currentScene);
  // if portal was entered, we need to update portals for new scene
  if(portalEntered) {
    // clear portals for old scene
    for(u32 portalIndex = 0; portalIndex < currentScene->portalCount; ++portalIndex) {
      clearFlags(&currentScene->portals[portalIndex].stateFlags); // clear all flags of previous currentScene
    }

    world->currentSceneIndex = portalSceneDestination;

    // update portals for new scene
    updatePortalsForScene(world->scenes + world->currentSceneIndex);
  }
}

void initGlobalShaders() {
  shaders_GLOBAL.singleColor = createShaderProgram(posVertexShaderFileLoc, singleColorFragmentShaderFileLoc);
  shaders_GLOBAL.stencil = createShaderProgram(posVertexShaderFileLoc, blackFragmentShaderFileLoc);
  shaders_GLOBAL.skybox = createShaderProgram(skyboxVertexShaderFileLoc, skyboxFragmentShaderFileLoc);
}

void deinitGlobalShaders() {
  deleteShaderPrograms(shaders_GLOBAL.shaders, ArrayCount(shaders_GLOBAL.shaders));
}

void cleanupScene(Scene* scene) {
  for(u32 entityIndex = 0; entityIndex < scene->entityCount; entityIndex++) {
    scene->entities[entityIndex] = {}; // zero out struct
  }
  scene->entityCount = 0;

  for(u32 portalIndex = 0; portalIndex < scene->portalCount; portalIndex++) {
    scene->portals[portalIndex] = {}; // zero out struct
  }
  scene->portalCount = 0;

  glDeleteTextures(1, &scene->skyboxTexture);
  scene->skyboxTexture = TEXTURE_ID_NO_TEXTURE;

  delete[] scene->title;
  if(scene->skyboxFileName != nullptr) { delete[] scene->skyboxFileName; }
  scene->title = nullptr;
  scene->skyboxFileName = nullptr;
}

void cleanupWorld(World* world) {
  for(u32 sceneIndex = 0; sceneIndex < world->sceneCount; sceneIndex++) {
    cleanupScene(world->scenes + sceneIndex);
  }

  deleteModels(world->models, world->modelCount);
  memset(world->models, 0, sizeof(Model) * world->modelCount);

  deleteShaderPrograms(world->shaders, world->shaderCount);

  *world = {0};
}

void initPlayer(Player* player) {
  player->boundingBox.diagonal = defaultPlayerDimensionInMeters;
//  player->boundingBox.min = {-(globalWorld.player.boundingBox.diagonal.x * 0.5f) - 2.8f, -3.0f - (globalWorld.player.boundingBox.diagonal.y * 0.5f), 0.0f};
  player->boundingBox.min = {-(globalWorld.player.boundingBox.diagonal.x * 0.5f), -12.0f - (globalWorld.player.boundingBox.diagonal.y * 0.5f), 0.0f};
}

void initCamera(Camera* camera, const Player& player) {
  vec3 firstPersonCameraInitPosition = calcPlayerViewingPosition(&player);
  vec3 firstPersonCameraInitFocus{0, 0, firstPersonCameraInitPosition.z};
  lookAt_FirstPerson(firstPersonCameraInitPosition, firstPersonCameraInitFocus, camera);
}

// TODO: We will no longer be loading worlds from json
// TODO: Cleanup the way the save files are organized.
void loadWorld(World* world, const char* saveJsonFile) {
  TimeFunction

  LOGI("Loading native Portal Scene...");

  SaveFormat saveFormat = originalWorld();

  size_t sceneCount = saveFormat.scenes.size();
  size_t modelCount = saveFormat.models.size();
  size_t shaderCount = saveFormat.shaders.size();

  u32 worldShaderIndices[ArrayCount(world->shaders)] = {};
  { // shaders
    for(u32 shaderIndex = 0; shaderIndex < shaderCount; shaderIndex++) {
      ShaderSaveFormat shaderSaveFormat = saveFormat.shaders[shaderIndex];
      assert(shaderSaveFormat.index < shaderCount);

      const char* noiseTexture = shaderSaveFormat.noiseTextureName.empty() ? nullptr : shaderSaveFormat.noiseTextureName.c_str();
      worldShaderIndices[shaderSaveFormat.index] = addNewShader(world, shaderSaveFormat.vertexName.c_str(), shaderSaveFormat.fragmentName.c_str(), noiseTexture);
    }
  }

  u32 worldModelIndices[ArrayCount(world->models)] = {};
  { // models
    for(u32 modelIndex = 0; modelIndex < modelCount; modelIndex++) {
      ModelSaveFormat modelSaveFormat = saveFormat.models[modelIndex];
      assert(modelSaveFormat.index < modelCount);
      worldModelIndices[modelSaveFormat.index] = addNewModel(world, modelSaveFormat.fileName.c_str());

      Model* model = world->models + worldModelIndices[modelSaveFormat.index];
      for(u32 meshIndex = 0; meshIndex < model->meshCount; meshIndex++) {
        Mesh* mesh = model->meshes + meshIndex;
        mesh->textureData.baseColor = modelSaveFormat.baseColor;
      }
    }
  }

  u32 worldSceneIndices[ArrayCount(world->scenes)] = {};
  { // scenes
    for(u32 sceneIndex = 0; sceneIndex < sceneCount; sceneIndex++) {
      SceneSaveFormat sceneSaveFormat = saveFormat.scenes[sceneIndex];
      size_t entityCount = sceneSaveFormat.entities.size();

      assert(sceneSaveFormat.index < sceneCount);
      worldSceneIndices[sceneSaveFormat.index] = addNewScene(world, sceneSaveFormat.title.c_str());
      Scene* scene = world->scenes + worldSceneIndices[sceneSaveFormat.index];
      scene->title = cStrAllocateAndCopy(sceneSaveFormat.title.c_str());

      if(!sceneSaveFormat.skyboxFileName.empty()) { // if we have a skybox...
        scene->skyboxFileName = cStrAllocateAndCopy(sceneSaveFormat.skyboxFileName.c_str());
        loadCubeMapTexture(scene->skyboxFileName, &scene->skyboxTexture);
      } else {
        scene->skyboxTexture = TEXTURE_ID_NO_TEXTURE;
      }

      for(u32 entityIndex = 0; entityIndex < entityCount; entityIndex++) {
        EntitySaveFormat entitySaveFormat = sceneSaveFormat.entities[entityIndex];
        addNewEntity(world, worldSceneIndices[sceneSaveFormat.index], worldModelIndices[entitySaveFormat.modelIndex],
                     entitySaveFormat.posXYZ, entitySaveFormat.scaleXYZ, entitySaveFormat.yaw,
                     worldShaderIndices[entitySaveFormat.shaderIndex], entitySaveFormat.flags);
      }

      size_t dirLightCount = sceneSaveFormat.directionalLights.size();
      for(u32 dirLightIndex = 0; dirLightIndex < dirLightCount; dirLightIndex++) {
        DirectionalLightSaveFormat dirLightSaveFormat = sceneSaveFormat.directionalLights[dirLightIndex];
        addNewDirectionalLight(world, worldSceneIndices[sceneSaveFormat.index], dirLightSaveFormat.color,
                               dirLightSaveFormat.power, dirLightSaveFormat.dirToSource);
      }

      size_t posLightCount = sceneSaveFormat.positionalLights.size();
      for(u32 posLightIndex = 0; posLightIndex < posLightCount; posLightIndex++) {
        PositionalLightSaveFormat posLightSaveFormat = sceneSaveFormat.positionalLights[posLightIndex];
        addNewPositionalLight(world, worldSceneIndices[sceneSaveFormat.index], posLightSaveFormat.color,
                               posLightSaveFormat.power, posLightSaveFormat.pos);
      }

      if(sceneSaveFormat.ambientLightSaveFormat.power != 0.0f) {
        adjustAmbientLight(world, worldSceneIndices[sceneSaveFormat.index],
                           sceneSaveFormat.ambientLightSaveFormat.color,
                           sceneSaveFormat.ambientLightSaveFormat.power);
      }
    }

    // we have to iterate over the worlds once more for portals, as the scene destination index requires
    // the other worlds to have been initialized
    for(u32 sceneIndex = 0; sceneIndex < sceneCount; sceneIndex++)
    {
      SceneSaveFormat sceneSaveFormat = saveFormat.scenes[sceneIndex];
      size_t portalCount = sceneSaveFormat.portals.size();
      assert(portalCount <= MAX_PORTALS);
      for (u32 portalIndex = 0; portalIndex < portalCount; portalIndex++)
      {
        PortalSaveFormat portalSaveFormat = sceneSaveFormat.portals[portalIndex];
        addPortal(world, worldSceneIndices[sceneSaveFormat.index], portalSaveFormat.centerXYZ, portalSaveFormat.normalXYZ, portalSaveFormat.dimensXY,
                  portalIndex + 1, worldSceneIndices[portalSaveFormat.destination]);
      }
    }
  }

  assert(saveFormat.startingSceneIndex < sceneCount);
  world->currentSceneIndex = worldSceneIndices[saveFormat.startingSceneIndex];

  return;
}

// TODO: Come up with better name
void updateSceneWindow(u32 width, u32 height) {
  globalWorld.fov = 45.f;
  globalWorld.aspect = f32(width) / f32(height);
  globalWorld.UBOs.projectionViewModelUbo.projection = perspective(globalWorld.fov, globalWorld.aspect, near, far);
  glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, projection), sizeof(mat4), &globalWorld.UBOs.projectionViewModelUbo.projection);
}

void initPortalScene() {
  TimeFunction

  glGenQueries(ArrayCount(portalQueryObjects_GLOBAL), portalQueryObjects_GLOBAL);

  initGlobalShaders();
  initGlobalVertexAtts();

  initPlayer(&globalWorld.player);
  initCamera(&globalWorld.camera, globalWorld.player);

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glLineWidth(3.0f);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_STENCIL_TEST);

  // UBOs
  {
    glGenBuffers(1, &globalWorld.UBOs.projectionViewModelUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.projectionViewModelUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionViewModelUBO), NULL, GL_STREAM_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, projectionViewModelUBOBindingIndex, globalWorld.UBOs.projectionViewModelUboId, 0, sizeof(ProjectionViewModelUBO));

    glGenBuffers(1, &globalWorld.UBOs.fragUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.fragUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(FragUBO), NULL, GL_STREAM_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, fragUBOBindingIndex, globalWorld.UBOs.fragUboId, 0, sizeof(FragUBO));

    glGenBuffers(1, &globalWorld.UBOs.lightUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.lightUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUBO), NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, lightUBOBindingIndex, globalWorld.UBOs.lightUboId, 0, sizeof(LightUBO));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }



  globalWorld.stopWatch = StopWatch();

  loadWorld(&globalWorld, "misc/noop_worlds/original_world.json");
}

void drawPortalScene() {
  // NOTE: input should be handled through handleInput(android_app* app, AInputEvent* event)
  globalWorld.stopWatch.lap();
  globalWorld.UBOs.fragUbo.time = globalWorld.stopWatch.totalInSeconds;

  vec3 playerCenter;
  vec3 playerViewPosition = calcPlayerViewingPosition(&globalWorld.player);

  // gather input
  // TODO: get input for frame to determine boolean values
  b32 sprintIsActive = false;
  b32 leftIsActive = false;
  b32 rightIsActive = false;
  b32 forwardIsActive = false;
  b32 backwardIsActive = false;
  vec2_f64 viewAngleDelta = {0.0, 0.0}; // TODO: Do we ever want to be able to change views?

  // gather input for movement and camera changes
  b32 lateralMovement = leftIsActive != rightIsActive;
  b32 forwardMovement = forwardIsActive != backwardIsActive;
  vec3 playerDelta{};
  if (lateralMovement || forwardMovement)
  {
    f32 playerMovementSpeed = sprintIsActive ? 8.0f : 4.0f;

    // Camera movement direction
    vec3 playerMovementDirection{};
    if (lateralMovement)
    {
      playerMovementDirection += rightIsActive ? globalWorld.camera.right : -globalWorld.camera.right;
    }

    if (forwardMovement)
    {
      playerMovementDirection += forwardIsActive ? globalWorld.camera.forward : -globalWorld.camera.forward;
    }

    playerMovementDirection = normalize(playerMovementDirection.x, playerMovementDirection.y, 0.0);
    playerDelta = playerMovementDirection * playerMovementSpeed * globalWorld.stopWatch.lapInSeconds;
  }

  // TODO: Do not apply immediately, check for collisions
  globalWorld.player.boundingBox.min += playerDelta;
  playerCenter = calcBoundingBoxCenterPosition(globalWorld.player.boundingBox);

  const f32 mouseDeltaMultConst = 0.0005f;
  updateCamera_FirstPerson(&globalWorld.camera, playerDelta, f32(-viewAngleDelta.y * mouseDeltaMultConst), f32(-viewAngleDelta.x * mouseDeltaMultConst));

  globalWorld.UBOs.projectionViewModelUbo.view = getViewMat(globalWorld.camera);

  // draw
  glStencilMask(0xFF);
  glStencilFunc(GL_ALWAYS, // stencil function always passes
                0x00, // reference
                0x00); // mask
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // universal matrices in UBO
  glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, offsetof(ProjectionViewModelUBO, model), &globalWorld.UBOs.projectionViewModelUbo);

  glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.fragUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FragUBO), &globalWorld.UBOs.fragUbo);

  updateEntities(&globalWorld);

  drawSceneWithPortals(&globalWorld);
}

void deinitPortalScene() {
  cleanupWorld(&globalWorld);
  deinitGlobalShaders();
  deinitGlobalVertexAtts();
  glDeleteQueries(ArrayCount(portalQueryObjects_GLOBAL), portalQueryObjects_GLOBAL);
}

void portalScene() {
  glGenQueries(ArrayCount(portalQueryObjects_GLOBAL), portalQueryObjects_GLOBAL);

  initGlobalShaders();
  initGlobalVertexAtts();

  initPlayer(&globalWorld.player);

  initCamera(&globalWorld.camera, globalWorld.player);

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glLineWidth(3.0f);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_STENCIL_TEST);

  // UBOs
  {
    glGenBuffers(1, &globalWorld.UBOs.projectionViewModelUboId);
    // allocate size for buffer
    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.projectionViewModelUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionViewModelUBO), NULL, GL_STREAM_DRAW);
    // attach buffer to ubo binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, projectionViewModelUBOBindingIndex, globalWorld.UBOs.projectionViewModelUboId, 0, sizeof(ProjectionViewModelUBO));

    glGenBuffers(1, &globalWorld.UBOs.fragUboId);
    // allocate size for buffer
    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.fragUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(FragUBO), NULL, GL_STREAM_DRAW);
    // attach buffer to ubo binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, fragUBOBindingIndex, globalWorld.UBOs.fragUboId, 0, sizeof(FragUBO));
  }

  globalWorld.stopWatch = StopWatch();

  while(true) // TODO: This is the render loop, it WILL need to exit
  {
    // NOTE: input should be handled through handleInput(android_app* app, AInputEvent* event)
    globalWorld.stopWatch.lap();
    globalWorld.UBOs.fragUbo.time = globalWorld.stopWatch.totalInSeconds;

    vec3 playerCenter;
    vec3 playerViewPosition = calcPlayerViewingPosition(&globalWorld.player);

    // gather input
    // TODO: get input for frame to determine boolean values
    b32 sprintIsActive = false;
    b32 leftIsActive = false;
    b32 rightIsActive = false;
    b32 forwardIsActive = false;
    b32 backwardIsActive = false;
    vec2_f64 viewAngleDelta = {0.0, 0.0}; // TODO: Do we ever want to be able to change views?

    // gather input for movement and camera changes
    b32 lateralMovement = leftIsActive != rightIsActive;
    b32 forwardMovement = forwardIsActive != backwardIsActive;
    vec3 playerDelta{};
    if (lateralMovement || forwardMovement)
    {
      f32 playerMovementSpeed = sprintIsActive ? 8.0f : 4.0f;

      // Camera movement direction
      vec3 playerMovementDirection{};
      if (lateralMovement)
      {
        playerMovementDirection += rightIsActive ? globalWorld.camera.right : -globalWorld.camera.right;
      }

      if (forwardMovement)
      {
        playerMovementDirection += forwardIsActive ? globalWorld.camera.forward : -globalWorld.camera.forward;
      }

      playerMovementDirection = normalize(playerMovementDirection.x, playerMovementDirection.y, 0.0);
      playerDelta = playerMovementDirection * playerMovementSpeed * globalWorld.stopWatch.lapInSeconds;
    }

    // TODO: Do not apply immediately, check for collisions
    globalWorld.player.boundingBox.min += playerDelta;
    playerCenter = calcBoundingBoxCenterPosition(globalWorld.player.boundingBox);

    const f32 mouseDeltaMultConst = 0.0005f;
    updateCamera_FirstPerson(&globalWorld.camera, playerDelta, f32(-viewAngleDelta.y * mouseDeltaMultConst), f32(-viewAngleDelta.x * mouseDeltaMultConst));

    globalWorld.UBOs.projectionViewModelUbo.view = getViewMat(globalWorld.camera);

    // draw
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, // stencil function always passes
                  0x00, // reference
                  0x00); // mask
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // universal matrices in UBO
    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, offsetof(ProjectionViewModelUBO, model), &globalWorld.UBOs.projectionViewModelUbo);

    glBindBuffer(GL_UNIFORM_BUFFER, globalWorld.UBOs.fragUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FragUBO), &globalWorld.UBOs.fragUbo);

    updateEntities(&globalWorld);

    drawSceneWithPortals(&globalWorld);

    // TODO: End of render loop, make sure to swap buffers and make sure we get input for next frame
  }
}