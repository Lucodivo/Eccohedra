#define PORTAL_BACKING_BOX_DEPTH 0.5f
#define MAX_PORTALS 8
#define MAX_SCENE_COUNT 8
#define STENCIL_MASK_BITS 8

struct PlayerPosition {
  struct {
    f32 theta;
    f32 radius;
    vec3 xyz;
  } pos; // use setters to ensure that these values are always in sync

  static PlayerPosition fromPolar(f32 theta, f32 radius) {
    PlayerPosition pp;
    pp.setPolarPos(theta, radius);
    return pp;
  }

  static PlayerPosition fromXYZ(f32 x, f32 y, f32 z){
    PlayerPosition pp;
    pp.setXYZPos(vec3{x, y, z});
    return pp;
  }

  static PlayerPosition fromXYZ(vec3 xyz){
    return fromXYZ(xyz[0], xyz[1], xyz[2]);
  }

  void setPolarPos(f32 theta, f32 radius) {
    pos.theta = theta;
    pos.radius = radius;
    float sinTheta = sin(pos.theta);
    float cosTheta = cos(pos.theta);
    pos.xyz = { cosTheta * pos.radius, sinTheta * pos.radius, 1.5f };
  }

  void setXYZPos(vec3 xyz) {
    pos.xyz = xyz;
    f32 magnitudeXY = magnitude(xyz.xy);
    pos.radius = magnitudeXY;
    f32 theta = acos(pos.xyz[0]/ magnitudeXY);
    pos.theta = xyz[1]> 0 ? theta : Tau32 - theta;
  }
};

struct Portal {
  vec2 normal;
  vec3 centerPosition;
  vec3 dimens;
  u32 sceneDestination;
  bool oneWay;
  bool transient;
  s32 backingModelIndex;
  s32 backingShaderIndex;
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
  vec4 ambientLightColorAndPower;
  GLuint skyboxTexture;
  u32 stencilMask;
  std::string title;
  std::string skyboxFileName;
};

struct SceneInput {
  bool active;
  f32 dx;
  f32 dy;
  f32 dSpan_pinch;
};

struct World
{
  PlayerPosition player;
  u32 currentSceneIndex;
  StopWatch stopWatch;
  Scene scenes[MAX_SCENE_COUNT];
  u32 sceneCount;
  Model models[128];
  u32 modelCount;
  struct {
    f32 fov;
    f32 aspect;
    u32 width;
    u32 height;
  } display;
  struct {
    ProjectionViewModelUBO projectionViewModelUbo;
    GLuint projectionViewModelUboId;
    FragUBO fragUbo;
    GLuint fragUboId;
    MultiLightUBO multiLightUbo;
    GLuint multiLightUboId;
  } UBOs;
  struct {
    SceneInput previousInputs[4];
    size_t previousInputIndex = ArrayCount(previousInputs) - 1;
    f32 flingVelocityX = 0.0f;
    f32 flingVelocityY = 0.0f;
  } inputHistory;
  ShaderProgram shaders[16];
  ShaderProgram skyboxShader;
  ShaderProgram stencilShader;
  CommonVertAtts commonVertAtts;
  u32 shaderCount;
};

const f32 near = 0.1f;
const f32 far = 200.0f;

void drawScene(World* world, const u32 sceneIndex, u32 stencilMask);
void drawSceneWithPortals(World* world, u32 sceneIndex, u32 stencilMask, u32 portalsDepth);

void addPortal(World* world, u32 sourceSceneIndex, const u32 destinationSceneIndex, PortalInfo* portalInfo, bool transient = false) {

  Scene* sourceScene = world->scenes + sourceSceneIndex;
  assert(ArrayCount(sourceScene->portals) > sourceScene->portalCount);

  Portal portal{};
  portal.dimens = portalInfo->dimensXYZ;
  portal.centerPosition = portalInfo->centerXYZ;
  portal.normal = portalInfo->normalXY;
  portal.oneWay = portalInfo->oneWay;
  portal.transient = transient;
  portal.sceneDestination = destinationSceneIndex;
  portal.backingModelIndex = portalInfo->backingModelIndex;
  portal.backingShaderIndex = portalInfo->backingShaderIndex;

  sourceScene->portals[sourceScene->portalCount++] = portal;
}

u32 addNewScene(World* world, const char* title) {
  assert(ArrayCount(world->scenes) > world->sceneCount);
  u32 sceneIndex = world->sceneCount;
  Scene* scene = world->scenes + sceneIndex;
  *scene = { 0 };
  scene->title = title;
  assert(world->sceneCount < STENCIL_MASK_BITS);
  scene->stencilMask = 1 << world->sceneCount;
  world->sceneCount++;
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
  entity->posXYZ = pos;
  entity->scaleXYZ = scale;
  entity->yaw = yaw;
  entity->shaderIndex = shaderIndex;
  entity->flags = entityTypeFlags;
  return sceneEntityIndex;
}

u32 addNewDirectionalLight(World* world, u32 sceneIndex, vec4 colorAndPower, vec3 lightToSource) {
  Scene* scene = world->scenes + sceneIndex;
  assert(scene->posLightCount + scene->dirLightCount < ArrayCount(scene->dirPosLightStack));
  u32 newLightIndex = scene->dirLightCount++;
  scene->dirPosLightStack[newLightIndex].colorAndPower = colorAndPower;
  scene->dirPosLightStack[newLightIndex].pos = normalize(lightToSource);
  return newLightIndex;
}

u32 addNewPositionalLight(World* world, u32 sceneIndex, vec4 colorAndPower, vec3 lightPos) {
  Scene* scene = world->scenes + sceneIndex;
  const u32 maxLights = ArrayCount(scene->dirPosLightStack);
  assert(scene->posLightCount + scene->dirLightCount < maxLights);
  u32 newLightIndex = maxLights - 1 - scene->posLightCount++;
  scene->dirPosLightStack[newLightIndex].colorAndPower = colorAndPower;
  scene->dirPosLightStack[newLightIndex].pos = lightPos;
  return newLightIndex;
}

void adjustAmbientLight(World* world, u32 sceneIndex, vec4 lightColorAndPower) {
  Scene* scene = world->scenes + sceneIndex;
  scene->ambientLightColorAndPower = {lightColorAndPower[0], lightColorAndPower[1], lightColorAndPower[2], lightColorAndPower[3]};
}

u32 addNewModel(World* world, const char* modelFileLoc) {
  u32 modelIndex = world->modelCount++;
  loadModelAsset(modelFileLoc, world->models + modelIndex);
  return modelIndex;
}

// returns true if it attempted to draw a portal
bool drawPortal(World* world, u32 sceneIndex, u32 portalIndex) {
  Portal* portal = world->scenes[sceneIndex].portals + portalIndex;
  vec2 portalToPlayer = world->player.pos.xyz.xy - portal->centerPosition.xy;
  vec2 playerViewDir = -world->player.pos.xyz.xy; // Player assumed to always face the origin
  bool playerInFrontOfPortal = dot(portalToPlayer, portal->normal) >= 0.0f;
  bool playerLookingInDirOfPortal = dot(playerViewDir, portal->normal) <= 0.0f;
  bool portalMightBeVisible = playerInFrontOfPortal && playerLookingInDirOfPortal;
  if(!portalMightBeVisible) { return false; }
  vec2 portalNormalPerp = vec2{portal->normal[1], -portal->normal[0]};
  f32 halfPortalWidth = portal->dimens[0] * 0.5f;
  bool portalMightBeInFocus = world->currentSceneIndex == sceneIndex &&
      abs(dot(portalToPlayer, portalNormalPerp)) < halfPortalWidth;

  glUseProgram(world->stencilShader.id);

  mat4 portalModelMat = quadModelMatrix(portal->centerPosition, Vec3(portal->normal, 0.0f), portal->dimens[0], portal->dimens[2]);
  portalModelMat = portalMightBeInFocus ?
      portalModelMat * scale_mat4(vec3{1.0f, PORTAL_BACKING_BOX_DEPTH, 1.0f}) * translate_mat4({0.0f, 0.5f, 0.0f}) :
      portalModelMat;
  VertexAtt* portalVertexAtt = portalMightBeInFocus ?
      world->commonVertAtts.cube(true, true) :
      world->commonVertAtts.quad(false);

  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &portalModelMat);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glUseProgram(world->stencilShader.id);
  glStencilMask(world->scenes[portal->sceneDestination].stencilMask);
  drawTriangles(portalVertexAtt);

  return true;
}

void drawPortals(World* world, const u32 sceneIndex, const u32 stencilMask, const u32 portalsDepth){
  if(portalsDepth <= 0) { return; }

  Scene* scene = world->scenes + sceneIndex;

  bool portalMightBeVisible[MAX_PORTALS];
  glStencilFunc(GL_EQUAL, 0xFF, stencilMask);
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  for(u32 portalIndex = 0; portalIndex < scene->portalCount; portalIndex++) {
    portalMightBeVisible[portalIndex] = drawPortal(world, sceneIndex, portalIndex);
  }
  glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

  // Draw portal worlds
  // We need to clear disable depth values so distant objects through the "portals" still get drawn
  // The portals themselves will still obey the depth of the scene, as the stencils have been rendered with depth in mind
  glClear(GL_DEPTH_BUFFER_BIT);

  for(u32 portalIndex = 0; portalIndex < scene->portalCount; portalIndex++) {
    if(!portalMightBeVisible[portalIndex]) { continue; }
    const Portal& portal = scene->portals[portalIndex];

    vec3 portalNormal_viewSpace = (world->UBOs.projectionViewModelUbo.view * Vec4(-portal.normal, 0.0f, 0.0f)).xyz;
    vec3 portalCenterPos_viewSpace = (world->UBOs.projectionViewModelUbo.view * Vec4(portal.centerPosition, 1.0f)).xyz;
    mat4 portalProjectionMat = world->display.width > world->display.height ?
                               obliquePerspective_fovHorz(world->display.fov, world->display.aspect, near, far, portalNormal_viewSpace, portalCenterPos_viewSpace) :
                               obliquePerspective(world->display.fov, world->display.aspect, near, far, portalNormal_viewSpace, portalCenterPos_viewSpace);

    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, projection), sizeof(mat4), &portalProjectionMat);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    drawScene(world, portal.sceneDestination, world->scenes[portal.sceneDestination].stencilMask);
    drawPortals(world, portal.sceneDestination, world->scenes[portal.sceneDestination].stencilMask, portalsDepth - 1);
  }
}

void drawScene(World* world, const u32 sceneIndex, u32 stencilMask) {
  glStencilFunc( GL_EQUAL, 0xFF, stencilMask);

  Scene* scene = world->scenes + sceneIndex;

  if(scene->skyboxTexture != TEXTURE_ID_NO_TEXTURE) { // draw skybox if one exists
    glUseProgram(world->skyboxShader.id);
    bindActiveTextureCubeMap(skyboxActiveTextureIndex, scene->skyboxTexture);
    setSamplerCube(world->skyboxShader.id, skyboxTexUniformName, skyboxActiveTextureIndex);
    mat4 identityMat4 = identity_mat4();
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &identityMat4);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    drawTriangles(world->commonVertAtts.cube(true));
  }

  // update scene light uniform buffer object
  {
    const u32 maxLights = ArrayCount(world->UBOs.multiLightUbo.dirPosLightStack);
    assert((scene->dirLightCount + scene->posLightCount) <= maxLights);

    // NOTE: The lights are on a single double ended array where directional lights are added to the beginning
    // and directional lights are added to the end.
    // TODO: If LightUniform and Light struct for class were the same we could do a simple memcpy
    world->UBOs.multiLightUbo.dirLightCount = scene->dirLightCount;
    for(u32 i = 0; i < scene->dirLightCount; ++i) {
      world->UBOs.multiLightUbo.dirPosLightStack[i].colorAndPower = scene->dirPosLightStack[i].colorAndPower;
      world->UBOs.multiLightUbo.dirPosLightStack[i].pos.xyz = scene->dirPosLightStack[i].pos;
      // TODO: W component of pos currently undefined and potentially dangerous. Determine if it can be used.
    }

    world->UBOs.multiLightUbo.posLightCount = scene->posLightCount;
    for(u32 i = 0; i < scene->posLightCount; ++i) {
      world->UBOs.multiLightUbo.dirPosLightStack[maxLights - i].colorAndPower = scene->dirPosLightStack[maxLights - i].colorAndPower;
      world->UBOs.multiLightUbo.dirPosLightStack[maxLights - i].pos.xyz = scene->dirPosLightStack[maxLights - i].pos;
      // TODO: W component of pos currently undefined and potentially dangerous. Determine if it can be used.
    }

    world->UBOs.multiLightUbo.ambientLight = scene->ambientLightColorAndPower;

    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.multiLightUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MultiLightUBO), &world->UBOs.multiLightUbo);
  }

  // draw entities
  for(u32 sceneEntityIndex = 0; sceneEntityIndex < scene->entityCount; ++sceneEntityIndex) {
    Entity* entity = &scene->entities[sceneEntityIndex];
    ShaderProgram shader = world->shaders[entity->shaderIndex];

    mat4 modelMatrix = scaleRotTrans_mat4(entity->scaleXYZ, vec3{0.0f, 0.0f, 1.0f}, entity->yaw, entity->posXYZ);

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
      if(mesh->textureData.baseColor[3] != 0.0f) {
        setUniform(shader.id, baseColorUniformName, mesh->textureData.baseColor.xyz);
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
  }

  // draw portal backs
  for(u32 portalIndex = 0; portalIndex < scene->portalCount; ++portalIndex) {
    const Portal& portal = scene->portals[portalIndex];
    if(portal.backingModelIndex == WORLD_INFO_NO_INDEX) { continue; }
    ShaderProgram shader = world->shaders[portal.backingShaderIndex];
    Model model = world->models[portal.backingModelIndex];

    f32 yaw = acos(-portal.normal[1]) * sign(portal.normal[0]);
    vec3 portalBackOffset = Vec3(-(0.5 * portal.dimens[1]) * portal.normal, 0.0f);
    mat4 modelMatrix = scaleRotTrans_mat4(portal.dimens, vec3{0.0f, 0.0f, 1.0f}, yaw, portal.centerPosition + portalBackOffset);

    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &modelMatrix);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUseProgram(shader.id);
    if(shader.noiseTextureId != TEXTURE_ID_NO_TEXTURE) {
      bindActiveTextureSampler2d(noiseActiveTextureIndex, shader.noiseTextureId);
      setSampler2D(shader.id, noiseTexUniformName, noiseActiveTextureIndex);
    }
    // TODO: Should some of this logic be moved to drawModel()?
    for(u32 meshIndex = 0; meshIndex < model.meshCount; ++meshIndex) {
      Mesh* mesh = model.meshes + meshIndex;
      if(mesh->textureData.baseColor[3] != 0.0f) {
        setUniform(shader.id, baseColorUniformName, mesh->textureData.baseColor.xyz);
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
  }
}

void drawSceneWithPortals(World* world, u32 sceneIndex, u32 stencilMask, u32 portalsDepth)
{
  // draw scene
  drawScene(world, sceneIndex, stencilMask);
  // draw portals
  drawPortals(world, world->currentSceneIndex, stencilMask, portalsDepth);
}

void updateEntities(World* world) {
  for(u32 sceneIndex = 0; sceneIndex < world->sceneCount; ++sceneIndex) {
    Scene* scene = world->scenes + sceneIndex;
    for(u32 entityIndex = 0; entityIndex < scene->entityCount; ++entityIndex) {
      Entity* entity = scene->entities + entityIndex;
      if(entity->flags & EntityType_Rotating) {
        entity->yaw += 30.0f * RadiansPerDegree * world->stopWatch.lapInSeconds;
        if(entity->yaw > Tau32) {
          entity->yaw -= Tau32;
        }
      }
    }
  }

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
}

void cleanupWorld(World* world) {
  for(u32 sceneIndex = 0; sceneIndex < world->sceneCount; sceneIndex++) {
    cleanupScene(world->scenes + sceneIndex);
  }

  deleteModels(world->models, world->modelCount);
  memset(world->models, 0, sizeof(Model) * world->modelCount);

  deleteShaderPrograms(world->shaders, world->shaderCount);
  deleteShaderPrograms(&world->stencilShader, 1);
  deleteShaderPrograms(&world->skyboxShader, 1);

  *world = {0};
}

void loadWorld(World* world) {
  // TODO: all of the indices work is superfluous in the current state of loading a world
  // TODO: further reduce the unnecessary logic
  LOGI("Loading native Portal Scene...");

  WorldInfo worldInfo = originalWorld();

  size_t sceneCount = worldInfo.scenes.size();
  size_t modelCount = worldInfo.models.size();
  size_t shaderCount = worldInfo.shaders.size();

  u32 worldShaderIndices[ArrayCount(world->shaders)] = {};
  { // shaders
    for(u32 shaderIndex = 0; shaderIndex < shaderCount; shaderIndex++) {
      ShaderInfo shaderInfo = worldInfo.shaders[shaderIndex];
      assert(shaderInfo.index < shaderCount);

      const char* noiseTexture = shaderInfo.noiseTextureName.empty() ? nullptr : shaderInfo.noiseTextureName.c_str();
      worldShaderIndices[shaderInfo.index] = addNewShader(world, shaderInfo.vertexName.c_str(), shaderInfo.fragmentName.c_str(), noiseTexture);
    }
  }

  u32 worldModelIndices[ArrayCount(world->models)] = {};
  { // models
    for(u32 modelIndex = 0; modelIndex < modelCount; modelIndex++) {
      ModelInfo modelInfo = worldInfo.models[modelIndex];
      assert(modelInfo.index < modelCount);
      worldModelIndices[modelInfo.index] = addNewModel(world, modelInfo.fileName.c_str());

      Model* model = world->models + worldModelIndices[modelInfo.index];
      for(u32 meshIndex = 0; meshIndex < model->meshCount; meshIndex++) {
        Mesh* mesh = model->meshes + meshIndex;
        mesh->textureData.baseColor = modelInfo.baseColor;
      }
    }
  }

  u32 worldSceneIndices[ArrayCount(world->scenes)] = {};
  { // scenes
    for(u32 sceneIndex = 0; sceneIndex < sceneCount; sceneIndex++) {
      SceneInfo sceneInfo = worldInfo.scenes[sceneIndex];
      size_t entityCount = sceneInfo.entities.size();

      assert(sceneInfo.index < sceneCount);
      worldSceneIndices[sceneInfo.index] = addNewScene(world, sceneInfo.title.c_str());
      Scene* scene = world->scenes + worldSceneIndices[sceneInfo.index];
      scene->title = sceneInfo.title;

      if(!sceneInfo.skyboxFileName.empty()) { // if we have a skybox...
        scene->skyboxFileName = sceneInfo.skyboxFileName;
        loadCubeMapTexture(scene->skyboxFileName.c_str(), &scene->skyboxTexture);
      } else {
        scene->skyboxTexture = TEXTURE_ID_NO_TEXTURE;
      }

      for(u32 entityIndex = 0; entityIndex < entityCount; entityIndex++) {
        Entity entity = sceneInfo.entities[entityIndex];
        addNewEntity(world, worldSceneIndices[sceneInfo.index], worldModelIndices[entity.modelIndex],
                     entity.posXYZ, entity.scaleXYZ, entity.yaw,
                     worldShaderIndices[entity.shaderIndex], entity.flags);
      }

      size_t dirLightCount = sceneInfo.directionalLights.size();
      for(u32 dirLightIndex = 0; dirLightIndex < dirLightCount; dirLightIndex++) {
        Light light = sceneInfo.directionalLights[dirLightIndex];
        addNewDirectionalLight(world, worldSceneIndices[sceneInfo.index], light.colorAndPower, light.dirToSource);
      }

      size_t posLightCount = sceneInfo.positionalLights.size();
      for(u32 posLightIndex = 0; posLightIndex < posLightCount; posLightIndex++) {
        Light light = sceneInfo.positionalLights[posLightIndex];
        addNewPositionalLight(world, worldSceneIndices[sceneInfo.index], light.colorAndPower, light.pos);
      }

      if(sceneInfo.ambientLightColorAndPower[3] != 0.0f) {
        adjustAmbientLight(world, worldSceneIndices[sceneInfo.index],
                           sceneInfo.ambientLightColorAndPower);
      }
    }

    // we have to iterate over the worlds once more for portals, as the scene destination index requires
    // the other worlds to have been initialized
    for(u32 sceneIndex = 0; sceneIndex < sceneCount; sceneIndex++)
    {
      SceneInfo sceneInfo = worldInfo.scenes[sceneIndex];
      size_t portalCount = sceneInfo.portals.size();
      assert(portalCount <= MAX_PORTALS);
      for (u32 portalIndex = 0; portalIndex < portalCount; portalIndex++)
      {
        PortalInfo portalInfo = sceneInfo.portals[portalIndex];
        addPortal(world, worldSceneIndices[sceneInfo.index], worldSceneIndices[portalInfo.destination], &portalInfo);
      }
    }
  }

  assert(worldInfo.startingSceneIndex < sceneCount);
  world->currentSceneIndex = worldSceneIndices[worldInfo.startingSceneIndex];

  return;
}

// TODO: Come up with better name
void updateSceneWindow(World* world, u32 width, u32 height) {
  world->display.fov = 45.f;
  world->display.width = width;
  world->display.height = height;
  world->display.aspect = f32(width) / f32(height);
  world->UBOs.projectionViewModelUbo.projection = width > height ?
      perspective_fovHorz(world->display.fov, world->display.aspect, near, far) :
      perspective(world->display.fov, world->display.aspect, near, far);
  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, projection), sizeof(mat4), &world->UBOs.projectionViewModelUbo.projection);
}

void initPortalScene(World* world) {
  initCommonVertexAtt(&world->commonVertAtts);

  world->player.setPolarPos(-PiOverTwo32, 12.0f);

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClearStencil(0x00);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glLineWidth(3.0f);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_STENCIL_TEST);

  // Universal shaders
  {
    world->stencilShader = createShaderProgram(posVertexShaderFileLoc, blackFragmentShaderFileLoc);
    world->skyboxShader = createShaderProgram(skyboxVertexShaderFileLoc, skyboxFragmentShaderFileLoc);
  }

  // UBOs
  {
    glGenBuffers(1, &world->UBOs.projectionViewModelUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ProjectionViewModelUBO), NULL, GL_STREAM_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, projectionViewModelUBOBindingIndex, world->UBOs.projectionViewModelUboId, 0, sizeof(ProjectionViewModelUBO));

    glGenBuffers(1, &world->UBOs.fragUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.fragUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(FragUBO), NULL, GL_STREAM_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, fragUBOBindingIndex, world->UBOs.fragUboId, 0, sizeof(FragUBO));

    glGenBuffers(1, &world->UBOs.multiLightUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.multiLightUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MultiLightUBO), NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, multiLightUBOBindingIndex, world->UBOs.multiLightUboId, 0, sizeof(MultiLightUBO));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &world->UBOs.multiLightUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.multiLightUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MultiLightUBO), NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, multiLightUBOBindingIndex, world->UBOs.multiLightUboId, 0, sizeof(MultiLightUBO));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  world->stopWatch = StopWatch();

  loadWorld(world);
}

/*
  NOTE: Although the scenes visual representation is all neatly stored in a structure that can be edited by the developer willy nilly
    And rendering things will work just as anticipated. That is currently not true for any sort of collision detection and resolution.
    This collision detection is hard baked for the current state of the worlds as of it's creation.

*/
void collisionDetectionAndCorrection(World* world, PlayerPosition desiredPosition) {
  PlayerPosition startingPlayerPos = world->player;
  PlayerPosition correctedPlayerPos = desiredPosition;
  const f32 outerBoundingSphereRadius = 75.0f;

  if(world->currentSceneIndex == 0) { // if gate scene...
    // TODO: check for collisions with the gate's columns
    const vec2 quarterFoldedXY = vec2{abs(correctedPlayerPos.pos.xyz[0]), abs(correctedPlayerPos.pos.xyz[1])};
    const vec2 columnXY = vec2{1.768f, 1.768 };
    const f32 columnRadius = 0.5f;
    const f32 columnRadiusSq = columnRadius * columnRadius;

    // intersecting column? Taking advantage of column symmetry.
    const vec2 foldedXY_columnOrigin = quarterFoldedXY - columnXY;
    if(magnitudeSquared(foldedXY_columnOrigin) < columnRadiusSq) { // intersecting with a column
      vec2 foldedXY_dirFromColumn = normalize(foldedXY_columnOrigin);
      vec2 foldedCorrection = (foldedXY_dirFromColumn * columnRadius) - foldedXY_columnOrigin;
      vec2 unfoldedCorrection = vec2{
          correctedPlayerPos.pos.xyz[0] > 0 ? foldedCorrection[0]: -foldedCorrection[0],
          correctedPlayerPos.pos.xyz[1] > 0 ? foldedCorrection[1]: -foldedCorrection[1]
      };
      correctedPlayerPos = PlayerPosition::fromXYZ(correctedPlayerPos.pos.xyz[0] + unfoldedCorrection[0], correctedPlayerPos.pos.xyz[1] + unfoldedCorrection[1], correctedPlayerPos.pos.xyz[2]);
    }
  }

  // correct potential collision with "shape"
  const f32 shapeBoundingSphereRadius = 1.5f;
  f32 newRadius = Max(correctedPlayerPos.pos.radius, shapeBoundingSphereRadius);
  correctedPlayerPos = PlayerPosition::fromPolar(correctedPlayerPos.pos.theta, newRadius);

  // prevent collision with portal backings
  Scene &scene = world->scenes[world->currentSceneIndex];
  for (u32 i = 0; i < scene.portalCount; i++) {
    Portal &portal = scene.portals[i];
    if(portal.backingModelIndex == WORLD_INFO_NO_INDEX) { continue; }
    // this logic assumes the portals normal never points even partially in the z dimension
    vec3 portalForward = Vec3(portal.normal, 0.0f); // forward == opening
    vec3 portalRight = cross(portalForward, vec3{0.f, 0.f, 1.f});
    vec3 deltaFromCenterLeftRight = (0.5f * portal.dimens[0] * portalRight);
    // check if the old position was not in front of the portal
    f32 maxPortalForwardDirection = dot(portal.centerPosition, portalForward);
    f32 startingPlayerInForwardDirection = dot(startingPlayerPos.pos.xyz, portalForward);
    bool playerWasInFrontOfPortal = startingPlayerInForwardDirection >= maxPortalForwardDirection;
    if (!playerWasInFrontOfPortal) { // only check collision detection if player was not in front of the portal.
      f32 portalBufferDepth = 0.5f;
      f32 minPortalForwardDirection =
          maxPortalForwardDirection - portal.dimens[1] - portalBufferDepth;
      f32 maxPortalRightDirection =
          dot(portal.centerPosition + deltaFromCenterLeftRight, portalRight) + portalBufferDepth;
      f32 minPortalRightDirection =
          maxPortalRightDirection - portal.dimens[0] - (2.0f * portalBufferDepth);
      f32 desiredPosInForwardDir = dot(correctedPlayerPos.pos.xyz, portalForward);
      f32 desiredPosInRightDir = dot(correctedPlayerPos.pos.xyz, portalRight);
      if (desiredPosInForwardDir > minPortalForwardDirection &&
          desiredPosInForwardDir < maxPortalForwardDirection &&
          desiredPosInRightDir > minPortalRightDirection &&
          desiredPosInRightDir < maxPortalRightDirection) {
        // desired position will cause a collision and is in need of correction
        // either push it out the back or out the side
        f32 correctionDeltaForward = minPortalForwardDirection - desiredPosInForwardDir;
        f32 correctionDeltaLeft = minPortalRightDirection - desiredPosInRightDir;
        f32 correctionDeltaRight = maxPortalRightDirection - desiredPosInRightDir;
        correctionDeltaRight =
            abs(correctionDeltaLeft) < abs(correctionDeltaRight) ? correctionDeltaLeft
                                                                 : correctionDeltaRight;
        if (abs(correctionDeltaForward) < abs(correctionDeltaRight)) {
          // push out the bottom
          correctedPlayerPos = PlayerPosition::fromXYZ(
              correctedPlayerPos.pos.xyz + (correctionDeltaForward * portalForward));
        } else {
          // push out the side
          correctedPlayerPos = PlayerPosition::fromXYZ(
              correctedPlayerPos.pos.xyz + (correctionDeltaRight * portalRight));
        }
      }
    }
  }

  correctedPlayerPos.pos.radius = Min(correctedPlayerPos.pos.radius, outerBoundingSphereRadius);
  world->player = correctedPlayerPos;

  // check portal collision
  Scene* currentScene = &world->scenes[world->currentSceneIndex];
  vec2 playerPosition = world->player.pos.xyz.xy;
  for(u32 portalIndex = 0; portalIndex < currentScene->portalCount; ++portalIndex) {
    Portal* portal = currentScene->portals + portalIndex;

    vec2 portalNormalPerp = vec2{portal->normal[1], -portal->normal[0]};
    vec2 portalCenterToEdgeDelta = (0.5f * portal->dimens[0] * portalNormalPerp);
    vec2 portalEdge1 = (portal->centerPosition.xy + portalCenterToEdgeDelta);
    vec2 portalEdge2 = (portal->centerPosition.xy - portalCenterToEdgeDelta);
    bool playerStartedInFrontOfPortal = similarDirection(startingPlayerPos.pos.xyz.xy - portal->centerPosition.xy, portal->normal);
    bool playerCrossedOverPortal = lineSegmentsIntersection(startingPlayerPos.pos.xyz.xy, playerPosition, portalEdge1, portalEdge2, nullptr);
    if(playerStartedInFrontOfPortal && playerCrossedOverPortal) {
      // NOTE: THIS SIGNIFIES A PLAYER HAS WONDERED TO THE OTHER SIDE OF A PORTAL

      // add a transient portal if there is not "other side" portal at the destination
      if(portal->oneWay) {
        PortalInfo transientPortal;
        transientPortal.normalXY = -portal->normal;
        transientPortal.centerXYZ = portal->centerPosition;
        transientPortal.dimensXYZ = portal->dimens;
        transientPortal.destination = world->currentSceneIndex;
        transientPortal.oneWay = false;
        transientPortal.backingModelIndex = portal->backingModelIndex;
        transientPortal.backingShaderIndex = portal->backingShaderIndex;
        addPortal(world, portal->sceneDestination, world->currentSceneIndex, &transientPortal, true);
      }

      // remove any transient portal in current scene. If any exist, it will be the last one.
      if(currentScene->portals[currentScene->portalCount - 1].transient) {
        currentScene->portalCount -= 1;
      }

      world->currentSceneIndex = portal->sceneDestination;
      break;
    }
  }
}

void updatePortalScene(World* world, SceneInput input) {
  const f32 thetaMultiplier = -2.0f;
  const f32 radiusMultiplier = -1.0f;
  const f32 flingDrag = .88f;
  const f32 flingMinVelocity = 0.0003f;
  const f32 flingThreshold = 0.004f;

  // NOTE: input should be handled through handleInput(android_app* app, AInputEvent* event)
  world->stopWatch.lap();
  world->UBOs.fragUbo.time = world->stopWatch.totalInSeconds;

  PlayerPosition& player = world->player;
  f32 thetaDelta, radiusDelta;
  if(input.active) {
    thetaDelta = thetaMultiplier * input.dx;
    radiusDelta = radiusMultiplier * (input.dy + input.dSpan_pinch);
    world->inputHistory.flingVelocityX = 0.0f;
    world->inputHistory.flingVelocityY = 0.0f;
  } else {
    const SceneInput& previousInput = world->inputHistory.previousInputs[world->inputHistory.previousInputIndex];
    if(previousInput.active) {
      // if last frame contained a movement, consider using highest values as a fling
      SceneInput maxPrevInput = world->inputHistory.previousInputs[0];
      for(size_t i = 1; i < ArrayCount(world->inputHistory.previousInputs); i++) {
        const SceneInput& prevInput = world->inputHistory.previousInputs[i];
        maxPrevInput.dx = abs(maxPrevInput.dx) > abs(prevInput.dx) ? maxPrevInput.dx : prevInput.dx;
        maxPrevInput.dy = abs(maxPrevInput.dy) > abs(prevInput.dy) ? maxPrevInput.dy : prevInput.dy;
        maxPrevInput.dSpan_pinch = abs(maxPrevInput.dSpan_pinch) > abs(prevInput.dSpan_pinch) ? maxPrevInput.dSpan_pinch : prevInput.dSpan_pinch;
      }
      if(((maxPrevInput.dx * maxPrevInput.dx) + (maxPrevInput.dy * maxPrevInput.dy) + (maxPrevInput.dSpan_pinch * maxPrevInput.dSpan_pinch)) > (flingThreshold * flingThreshold)) {
        world->inputHistory.flingVelocityX = maxPrevInput.dx;
        world->inputHistory.flingVelocityY = maxPrevInput.dy + maxPrevInput.dSpan_pinch;
      }
    }
    thetaDelta = thetaMultiplier * world->inputHistory.flingVelocityX;
    radiusDelta = radiusMultiplier * world->inputHistory.flingVelocityY;
    world->inputHistory.flingVelocityX *= flingDrag;
    world->inputHistory.flingVelocityY *= flingDrag;
    if(abs(world->inputHistory.flingVelocityX) < flingMinVelocity) { world->inputHistory.flingVelocityX = 0.0f; }
    if(abs(world->inputHistory.flingVelocityY) < flingMinVelocity) { world->inputHistory.flingVelocityY = 0.0f; }
  }
  world->inputHistory.previousInputIndex = (world->inputHistory.previousInputIndex + 1) % ArrayCount(world->inputHistory.previousInputs);
  world->inputHistory.previousInputs[world->inputHistory.previousInputIndex] = input;

  f32 newTheta = player.pos.theta + thetaDelta;
  f32 newRadius = player.pos.radius + (radiusDelta * player.pos.radius);
  if(newTheta > Tau32) { newTheta -= Tau32; }
  if(newTheta < 0) { newTheta += Tau32; }

  PlayerPosition desiredPosition = PlayerPosition::fromPolar(newTheta, newRadius);
  collisionDetectionAndCorrection(world, desiredPosition);

  updateEntities(world);
}

void drawPortalScene(World* world) {
  Camera frameCamera;
  vec3 focusPoint = vec3{0.0f, 0.0f, 1.5f};
  lookAt_FirstPerson(world->player.pos.xyz, focusPoint, &frameCamera);
  mat4 cameraMat = getViewMat(frameCamera);
  world->UBOs.projectionViewModelUbo.view = cameraMat;

  // draw
  glStencilMask(0xFF);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // universal matrices in UBO
  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, offsetof(ProjectionViewModelUBO, model), &world->UBOs.projectionViewModelUbo);

  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.fragUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FragUBO), &world->UBOs.fragUbo);

  drawSceneWithPortals(world, world->currentSceneIndex, 0x00, 2);
}

void deinitPortalScene(World* world) {
  cleanupWorld(world);
  deinitCommonVertexAtts(&world->commonVertAtts);
}