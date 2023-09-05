#pragma once

internal_func u32 loadShader(const char* shaderPath, GLenum shaderType);

ShaderProgram createShaderProgram(const char* vertexPath, const char* fragmentPath, const char* noiseTexture = nullptr) {
  ShaderProgram shaderProgram{};
  shaderProgram.vertexFileName = cStrAllocateAndCopy(vertexPath);
  shaderProgram.fragmentFileName = cStrAllocateAndCopy(fragmentPath);
  shaderProgram.vertexShader = loadShader(shaderProgram.vertexFileName, GL_VERTEX_SHADER);
  shaderProgram.fragmentShader = loadShader(shaderProgram.fragmentFileName, GL_FRAGMENT_SHADER);

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
    shaderProgram.noiseTextureFileName = cStrAllocateAndCopy(noiseTexture);
    load2DTexture(shaderProgram.noiseTextureFileName, &shaderProgram.noiseTextureId);
  }

  return shaderProgram;
}

void deleteShaderProgram(ShaderProgram* shaderProgram)
{
  // delete the shaders
  delete[] shaderProgram->vertexFileName;
  delete[] shaderProgram->fragmentFileName;

  glDeleteShader(shaderProgram->vertexShader);
  glDeleteShader(shaderProgram->fragmentShader);
  glDeleteProgram(shaderProgram->id);

  if(shaderProgram->noiseTextureFileName != nullptr) {
    delete[] shaderProgram->noiseTextureFileName;
    glDeleteTextures(1, &shaderProgram->noiseTextureId);
  }

  *shaderProgram = {}; // clear to zero
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
                     mat->val); // pointer to float values
}

inline void setUniform(GLuint shaderId, const std::string& name, const mat4* matArray, const u32 arraySize)
{
  glUniformMatrix4fv(glGetUniformLocation(shaderId, name.c_str()),
                     arraySize, // count
                     GL_FALSE, // transpose: swap columns and rows (true or false)
                     matArray->val); // pointer to float values
}

inline void setUniform(GLuint shaderId, const std::string& name, const float* floatArray, const u32 arraySize)
{
  glUniform1fv(glGetUniformLocation(shaderId, name.c_str()), arraySize, floatArray);
}

inline void setUniform(GLuint shaderId, const std::string& name, const vec2& vector2)
{
  setUniform(shaderId, name, vector2.x, vector2.y);
}

inline void setUniform(GLuint shaderId, const std::string& name, const vec3& vector3)
{
  setUniform(shaderId, name, vector3.x, vector3.y, vector3.z);
}

inline void setUniform(GLuint shaderId, const std::string& name, const vec4& vector4)
{
  setUniform(shaderId, name, vector4.x, vector4.y, vector4.z, vector4.w);
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
internal_func GLuint loadShader(const char* shaderPath, GLenum shaderType) {
  std::string shaderTypeStr;
  if(shaderType == GL_VERTEX_SHADER) {
    shaderTypeStr = "VERTEX";
  } else if(shaderType == GL_FRAGMENT_SHADER){
    shaderTypeStr = "FRAGMENT";
  }

  Asset shaderAsset = Asset(shaderPath);
  if(!shaderAsset.success()) {
    LOGE("ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ - %s", shaderPath);
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
    LOGE("ERROR::SHADER::%s::COMPILATION_FAILED - %s\n%s", shaderTypeStr.c_str(), shaderPath, infoLog);
    return 0;
  }

  return shader;
}