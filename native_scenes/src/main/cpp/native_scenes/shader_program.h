#pragma once

internal_func u32 loadShader(const char* shaderFileName, GLenum shaderType);

ShaderProgram createShaderProgram(const char* vertexPath, const char* fragmentPath, const char* noiseTexture = nullptr) {
  ShaderProgram shaderProgram{};
  shaderProgram.vertexFileName = vertexPath;
  shaderProgram.fragmentFileName = fragmentPath;
  shaderProgram.vertexShader = loadShader(shaderProgram.vertexFileName.c_str(), GL_VERTEX_SHADER);
  shaderProgram.fragmentShader = loadShader(shaderProgram.fragmentFileName.c_str(), GL_FRAGMENT_SHADER);

  // shader program
  shaderProgram.id = glCreateProgram(); // NOTE: returns 0 if error occurs when creating program
  glAttachShader(shaderProgram.id, shaderProgram.vertexShader);
  glAttachShader(shaderProgram.id, shaderProgram.fragmentShader);
  glLinkProgram(shaderProgram.id);

  s32 linkSuccess;
  glGetProgramiv(shaderProgram.id, GL_LINK_STATUS, &linkSuccess);
  if (!linkSuccess)
  {
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram.id, 512, NULL, infoLog);
    LOGE("ERROR::PROGRAM::SHADER::LINK_FAILED\n%s", infoLog);
    exit(-1);
  }

  glDetachShader(shaderProgram.id, shaderProgram.vertexShader);
  glDetachShader(shaderProgram.id, shaderProgram.fragmentShader);

  if(noiseTexture != nullptr) {
    shaderProgram.noiseTextureFileName = noiseTexture;
    load2DTexture(shaderProgram.noiseTextureFileName.c_str(), &shaderProgram.noiseTextureId);
  }

  return shaderProgram;
}

void deleteShaderPrograms(ShaderProgram* shaderPrograms, u32 count) {
  for(u32 i = 0; i < count; i++) {
    ShaderProgram* shader = shaderPrograms + i;

    // delete the shaders
    glDeleteShader(shader->vertexShader);
    glDeleteShader(shader->fragmentShader);
    glDeleteProgram(shader->id);

    if(!shader->noiseTextureFileName.empty()) {
      glDeleteTextures(1, &shader->noiseTextureId);
    }

    *shader = {};
  }
}

// utility uniform functions
inline void setUniform(GLuint shaderId, const std::string& name, bool value)
{
  glUniform1i(glGetUniformLocation(shaderId, name.c_str()), (int)value);
}

inline void setUniform(GLuint shaderId, const std::string& name, s32 value)
{
  glUniform1i(glGetUniformLocation(shaderId, name.c_str()), value);
}

inline void setUniform(GLuint shaderId, const std::string& name, u32 value)
{
  glUniform1ui(glGetUniformLocation(shaderId, name.c_str()), value);
}

inline void setSamplerCube(GLuint shaderId, const std::string& name, GLint activeTextureIndex) {
  glUniform1i(glGetUniformLocation(shaderId, name.c_str()), activeTextureIndex);
}

inline void setSampler2D(GLuint shaderId, const std::string& name, GLint activeTextureIndex) {
  glUniform1i(glGetUniformLocation(shaderId, name.c_str()), activeTextureIndex);
}

inline void setUniform(GLuint shaderId, const std::string& name, f32 value)
{
  glUniform1f(glGetUniformLocation(shaderId, name.c_str()), value);
}

inline void setUniform(GLuint shaderId, const std::string& name, f32 value1, f32 value2)
{
  glUniform2f(glGetUniformLocation(shaderId, name.c_str()), value1, value2);
}

inline void setUniform(GLuint shaderId, const std::string& name, f32 value1, f32 value2, f32 value3)
{
  glUniform3f(glGetUniformLocation(shaderId, name.c_str()), value1, value2, value3);
}

inline void setUniform(GLuint shaderId, const std::string& name, f32 value1, f32 value2, f32 value3, f32 value4)
{
  glUniform4f(glGetUniformLocation(shaderId, name.c_str()), value1, value2, value3, value4);
}

inline void setUniform(GLuint shaderId, const std::string& name, const mat4* mat)
{
  glUniformMatrix4fv(glGetUniformLocation(shaderId, name.c_str()),
                     1, // count
                     GL_FALSE, // transpose: swap columns and rows (true or false)
                     mat->values); // pointer to float values
}

inline void setUniform(GLuint shaderId, const std::string& name, const mat4* matArray, const u32 arraySize)
{
  glUniformMatrix4fv(glGetUniformLocation(shaderId, name.c_str()),
                     arraySize, // count
                     GL_FALSE, // transpose: swap columns and rows (true or false)
                     matArray->values); // pointer to float values
}

inline void setUniform(GLuint shaderId, const std::string& name, const float* floatArray, const u32 arraySize)
{
  glUniform1fv(glGetUniformLocation(shaderId, name.c_str()), arraySize, floatArray);
}

inline void setUniform(GLuint shaderId, const std::string& name, const vec2& vector2)
{
  setUniform(shaderId, name, vector2[0], vector2[1]);
}

inline void setUniform(GLuint shaderId, const std::string& name, const vec3& vector3)
{
  setUniform(shaderId, name, vector3[0], vector3[1], vector3[2]);
}

inline void setUniform(GLuint shaderId, const std::string& name, const vec4& vector4)
{
  setUniform(shaderId, name, vector4[0], vector4[1], vector4[2], vector4[3]);
}

inline void bindBlockIndex(GLuint shaderId, const std::string& name, u32 index)
{
  u32 blockIndex = glGetUniformBlockIndex(shaderId, name.c_str());
  glUniformBlockBinding(shaderId, blockIndex, index);
}

/*
 * parameters:
 *  - shaderType can be GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, or GL_GEOMETRY_SHADER
 * returns:
 *  - Shader id
 *    - 0 is returned on error to load shader
 */
internal_func GLuint loadShader(const char* shaderFileName, GLenum shaderType) {
  std::string shaderTypeStr;
  if(shaderType == GL_VERTEX_SHADER) {
    shaderTypeStr = "VERTEX";
  } else if(shaderType == GL_FRAGMENT_SHADER){
    shaderTypeStr = "FRAGMENT";
  }

  std::string shaderDir = "shaders/";
  std::string assetPath = shaderDir + shaderFileName;
  Asset shaderAsset = Asset(assetManager_GLOBAL, assetPath.c_str());
  if(!shaderAsset.success()) {
    LOGE("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ - %s", shaderFileName);
    return 0;
  }

  GLuint shader = glCreateShader(shaderType);
  const GLint shaderLengths[] = { (GLint)shaderAsset.bufferLengthInBytes };
  const GLchar* shaderCode = (const GLchar *)shaderAsset.buffer;
  glShaderSource(shader, 1, &shaderCode, shaderLengths);
  glCompileShader(shader);

  s32 shaderSuccess;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderSuccess);
  if (shaderSuccess != GL_TRUE)
  {
    char infoLog[1024];
    glGetShaderInfoLog(shader, ArrayCount(infoLog), NULL, infoLog);
    LOGE("ERROR::SHADER::%s::COMPILATION_FAILED - %s\n%s", shaderTypeStr.c_str(), shaderFileName, infoLog);
    return 0;
  }

  return shader;
}