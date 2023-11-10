#define PORTAL_BACKING_BOX_DEPTH 0.5f
#define MAX_PORTALS 4

struct Player {
  struct {
    f32 theta;
    f32 radius;
    vec3 xyz;
  } pos; // use setters to ensure that these values are always in sync

  void setPolarPos(f32 theta, f32 radius) {
    pos.theta = theta;
    pos.radius = radius;
    float sinTheta = sin(pos.theta);
    float cosTheta = cos(pos.theta);
    pos.xyz = { cosTheta * pos.radius, sinTheta * pos.radius, 1.75f};
  }

  void setXYZPos(vec3 xyz) {
    pos.xyz = xyz;
    f32 magnitudeXY = magnitude(xyz.xy);
    pos.radius = magnitudeXY;
    f32 theta = acos(pos.xyz[0]/ magnitudeXY);
    pos.theta = xyz[1]> 0 ? theta : Tau32 - theta;
  }
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
  std::string title;
  std::string skyboxFileName;
};

struct SceneInput {
  f32 x;
  f32 y;
};

struct World
{
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
  ShaderProgram singleColorShader;
  ShaderProgram skyboxShader;
  ShaderProgram stencilShader;
  GLuint portalQueryObjects[MAX_PORTALS];
  CommonVertAtts commonVertAtts;
  u32 shaderCount;
};

const f32 near = 0.1f;
const f32 far = 200.0f;

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
  scene->dirPosLightStack[newLightIndex].color = { lightColor[0], lightColor[1], lightColor[2], lightPower};
  scene->dirPosLightStack[newLightIndex].pos = normalize(lightToSource);
  return newLightIndex;
}

u32 addNewPositionalLight(World* world, u32 sceneIndex, vec3 lightColor, f32 lightPower, vec3 lightPos) {
  Scene* scene = world->scenes + sceneIndex;
  const u32 maxLights = ArrayCount(scene->dirPosLightStack);
  assert(scene->posLightCount + scene->dirLightCount < maxLights);
  u32 newLightIndex = maxLights - 1 - scene->posLightCount++;
  scene->dirPosLightStack[newLightIndex].color = { lightColor[0], lightColor[1], lightColor[2], lightPower};
  scene->dirPosLightStack[newLightIndex].pos = lightPos;
  return newLightIndex;
}

void adjustAmbientLight(World* world, u32 sceneIndex, vec3 lightColor, f32 lightPower) {
  Scene* scene = world->scenes + sceneIndex;
  scene->ambientLight = { lightColor[0], lightColor[1], lightColor[2], lightPower};
}

u32 addNewModel(World* world, const char* modelFileLoc) {
  u32 modelIndex = world->modelCount++;
  loadModelAsset(modelFileLoc, world->models + modelIndex);
  return modelIndex;
}

void drawTrianglesWireframe(World* world, const VertexAtt* vertexAtt) {
  // TODO: This will not work at all in a general case
  // TODO: It only currently works because the wireframe shapes are the first things we draw in each scene
  // TODO: can't just disable the depth test whenever
  glUseProgram(world->singleColorShader.id);
  setUniform(world->singleColorShader.id, "baseColor", vec3{0.0f, 0.0f, 0.0f});
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  drawLines(vertexAtt); // TODO: This probably does not work and should be monitored
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
}

mat4 calcBoxStencilModelMatFromPortalModelMat(const mat4& portalModelMat) {
  return portalModelMat * scale_mat4(vec3{1.0f, PORTAL_BACKING_BOX_DEPTH, 1.0f}) * translate_mat4(-cubeFaceNegativeYCenter);
}

void drawPortal(World* world, Portal* portal) {
  glUseProgram(world->stencilShader.id);

  // NOTE: Stencil function Example
  // GL_LEQUAL
  // Passes if ( ref & mask ) <= ( stencil & mask )
  glStencilFunc(GL_EQUAL, // func
                0xFF, // ref
                0x00); // mask // Only draw portals where the stencil is cleared
  glStencilOp(GL_KEEP, // action when stencil fails
              GL_KEEP, // action when stencil passes but depth fails
              GL_REPLACE); // action when both stencil and depth pass

  mat4 portalModelMat = quadModelMatrix(portal->centerPosition, portal->normal, portal->dimens[0], portal->dimens[1]);
  VertexAtt* portalVertexAtt = world->commonVertAtts.quad(false);
  if(flagIsSet(portal->stateFlags, PortalState_InFocus)) {
    portalModelMat = calcBoxStencilModelMatFromPortalModelMat(portalModelMat);
    portalVertexAtt = world->commonVertAtts.cube(true, true);
  }

  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, model), sizeof(mat4), &portalModelMat);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glUseProgram(world->stencilShader.id);
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
    glBeginQuery(GL_ANY_SAMPLES_PASSED, world->portalQueryObjects[portalIndex]);
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

    if(entity->typeFlags & EntityType_Wireframe) { // wireframes should be drawn on top default mesh
      for(u32 meshIndex = 0; meshIndex < model.meshCount; ++meshIndex) {
        Mesh* mesh = model.meshes + meshIndex;
        drawTrianglesWireframe(world, &mesh->vertexAtt);
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
  vec3 playerViewPosition = world->player.pos.xyz;
  b32 portalEntered = false;
  u32 portalSceneDestination;
  auto updatePortalsForScene = [playerViewPosition, &portalEntered, &portalSceneDestination, &world, &currentScene](Scene* scene) {
    for(u32 portalIndex = 0; portalIndex < scene->portalCount; ++portalIndex) {
      Portal* portal = scene->portals + portalIndex;

      vec3 portalCenterToPlayerView = playerViewPosition - portal->centerPosition;
      b32 portalFacingCamera = similarDirection(portal->normal, playerViewPosition - portal->centerPosition) ? PortalState_FacingCamera : false;

      b32 portalWasInFocus = flagIsSet(portal->stateFlags, PortalState_InFocus);
      vec3 viewPositionPerpendicularToPortal = perpendicularTo(portalCenterToPlayerView, portal->normal);
      f32 widthDistFromCenter = magnitude(viewPositionPerpendicularToPortal.xy);
      f32 heightDistFromCenter = viewPositionPerpendicularToPortal[2];
      b32 viewerInsideDimens = widthDistFromCenter < (portal->dimens[0]* 0.5f) && heightDistFromCenter < (portal->dimens[1]* 0.5f);

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
  deleteShaderPrograms(&world->singleColorShader, 1);
  deleteShaderPrograms(&world->stencilShader, 1);
  deleteShaderPrograms(&world->skyboxShader, 1);

  glDeleteQueries(ArrayCount(world->portalQueryObjects), world->portalQueryObjects);

  *world = {0};
}

// TODO: Cleanup the way the save files are organized.
void loadWorld(World* world) {
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
      scene->title = sceneSaveFormat.title;

      if(!sceneSaveFormat.skyboxFileName.empty()) { // if we have a skybox...
        scene->skyboxFileName = sceneSaveFormat.skyboxFileName;
        loadCubeMapTexture(scene->skyboxFileName.c_str(), &scene->skyboxTexture);
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
void updateSceneWindow(World* world, u32 width, u32 height) {
  world->fov = 45.f;
  world->aspect = f32(width) / f32(height);
  world->UBOs.projectionViewModelUbo.projection = perspective(world->fov, world->aspect, near, far);
  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(ProjectionViewModelUBO, projection), sizeof(mat4), &world->UBOs.projectionViewModelUbo.projection);
}

void initPortalScene(World* world) {
  TimeFunction

  glGenQueries(ArrayCount(world->portalQueryObjects), world->portalQueryObjects);

  initCommonVertexAtt(&world->commonVertAtts);

  world->player.setPolarPos(-PiOverTwo32, 12.0f);

  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glLineWidth(3.0f);
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_STENCIL_TEST);

  // Universal shaders
  {
    world->singleColorShader = createShaderProgram(posVertexShaderFileLoc, singleColorFragmentShaderFileLoc);
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

    glGenBuffers(1, &world->UBOs.lightUboId);
    glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.lightUboId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LightUBO), NULL, GL_DYNAMIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, lightUBOBindingIndex, world->UBOs.lightUboId, 0, sizeof(LightUBO));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  world->stopWatch = StopWatch();

  loadWorld(world);
}

void updatePlayerCollisions(World* world) {
  Player& player = world->player;
  // collision detection & resolution
  if(world->currentSceneIndex == 0) { // if gate scene...
    // TODO: check for collisions with the gate's columns
    const vec2 quarterFoldedXY = vec2{abs(player.pos.xyz[0]), abs(player.pos.xyz[1])};
    const vec2 columnXY = vec2{1.768f, 1.768 };
    const f32 columnRadius = 0.5f;
    const f32 columnRadiusSq = columnRadius * columnRadius;

    // intersecting column? Taking advantage of column symmetry.
    const vec2 foldedXY_columnOrigin = quarterFoldedXY - columnXY;
    if(magnitudeSquared(foldedXY_columnOrigin) < columnRadiusSq) { // intersecting with a column
      vec2 foldedXY_dirFromColumn = normalize(foldedXY_columnOrigin);
      vec2 foldedCorrection = (foldedXY_dirFromColumn * columnRadius) - foldedXY_columnOrigin;
      vec2 unfoldedCorrection = vec2{
          player.pos.xyz[0] > 0 ? foldedCorrection[0]: -foldedCorrection[0],
          player.pos.xyz[1] > 0 ? foldedCorrection[1]: -foldedCorrection[1]
      };
      player.setXYZPos(vec3{player.pos.xyz[0] + unfoldedCorrection[0], player.pos.xyz[1] + unfoldedCorrection[1], player.pos.xyz[2]});
    }
  }
}

void updatePortalScene(World* world, SceneInput input) {
  // NOTE: input should be handled through handleInput(android_app* app, AInputEvent* event)
  world->stopWatch.lap();
  world->UBOs.fragUbo.time = world->stopWatch.totalInSeconds;

  Player& player = world->player;
  f32 thetaDelta = -(2.0 * input.x);
  f32 newTheta = player.pos.theta + thetaDelta;
  f32 radiusDelta = -(10.0f * input.y);
  f32 newRadius = player.pos.radius + radiusDelta;

  if(newTheta > Tau32) { newTheta -= Tau32; }
  if(newTheta < 0) { newTheta += Tau32; }
  newRadius = Max(newRadius, 1.5f);
  player.setPolarPos(newTheta, newRadius);

  updatePlayerCollisions(world);
  updateEntities(world);
}

void drawPortalScene(World* world) {
  Camera frameCamera;
  vec3 focusPoint = vec3{0.0f, 0.0f, 1.75f};
  lookAt_FirstPerson(world->player.pos.xyz, focusPoint, &frameCamera);
  mat4 cameraMat = getViewMat(frameCamera);
  world->UBOs.projectionViewModelUbo.view = cameraMat;

  // draw
  glStencilMask(0xFF);
  glStencilFunc(GL_ALWAYS, // stencil function always passes
                0x00, // reference
                0x00); // mask
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // universal matrices in UBO
  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.projectionViewModelUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, offsetof(ProjectionViewModelUBO, model), &world->UBOs.projectionViewModelUbo);

  glBindBuffer(GL_UNIFORM_BUFFER, world->UBOs.fragUboId);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FragUBO), &world->UBOs.fragUbo);

  drawSceneWithPortals(world);
}

void deinitPortalScene(World* world) {
  cleanupWorld(world);
  deinitCommonVertexAtts(&world->commonVertAtts);
}