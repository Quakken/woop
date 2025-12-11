#include "shader.hpp"
#include "log.hpp"
#include <fstream>
#include <sstream>

namespace woop {
Shader::Shader(const std::string& vert_src, const std::string& frag_src) {
  if (vert_src.empty() || frag_src.empty())
    throw ShaderException(
        ShaderException::Type::InvalidSource,
        "Attempting to create shader with empty source file.");
  program = compile(vert_src, frag_src);
}
Shader::Shader(Shader&& other) {
  program = other.program;
  invalid = false;
  other.invalid = true;
}
Shader::~Shader() {
  if (is_valid())
    glDeleteShader(program);
}

Shader& Shader::operator=(Shader&& other) {
  if (is_valid())
    glDeleteShader(program);
  program = other.program;
  invalid = other.invalid;
  other.invalid = true;
  return *this;
}

Shader Shader::from_file(const std::filesystem::path& vert_path,
                         const std::filesystem::path& frag_path) {
  std::ifstream vert_file(vert_path);
  std::ifstream frag_file(frag_path);
  if (!vert_file.is_open())
    throw ShaderException(ShaderException::Type::InvalidPath,
                          "Could not open vertex shader at given path.");
  if (!frag_file.is_open())
    throw ShaderException(ShaderException::Type::InvalidPath,
                          "Could not open fragment shader at given path.");
  std::ostringstream vert_stream;
  std::ostringstream frag_stream;
  vert_stream << vert_file.rdbuf();
  frag_stream << frag_file.rdbuf();
  std::string vert_src = vert_stream.str();
  std::string frag_src = frag_stream.str();
  return Shader(vert_src, frag_src);
}

void Shader::set_uniform(const std::string& name, bool v) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform1i(location, v);
}
void Shader::set_uniform(const std::string& name, int v) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform1i(location, v);
}
void Shader::set_uniform(const std::string& name, unsigned v) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform1ui(location, v);
}
void Shader::set_uniform(const std::string& name, float v) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform1f(location, v);
}
void Shader::set_uniform(const std::string& name, const glm::vec2& vec) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform2f(location, vec.x, vec.y);
}
void Shader::set_uniform(const std::string& name, const glm::vec3& vec) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform3f(location, vec.x, vec.y, vec.z);
}
void Shader::set_uniform(const std::string& name, const glm::vec4& vec) const {
  use();
  GLint location = glGetUniformLocation(program, name.c_str());
  glUniform4f(location, vec.x, vec.y, vec.z, vec.w);
}

void Shader::use() const {
  if (is_valid())
    glUseProgram(program);
  else
    throw ShaderException(ShaderException::Type::InvalidUse,
                          "Attempting to use invalidated shader.");
}

GLuint Shader::compile(const std::string& vert_src,
                       const std::string& frag_src) {
  const char* vert_src_raw = vert_src.c_str();
  const char* frag_src_raw = frag_src.c_str();

  GLuint vert = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vert, 1, &vert_src_raw, NULL);
  glCompileShader(vert);

  try {
    check_shader_compile_success(vert);
  } catch (ShaderException& e) {
    log_error("Could not compile vertex shader!");
    glDeleteShader(vert);
    throw e;
  }

  GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(frag, 1, &frag_src_raw, NULL);
  glCompileShader(frag);

  try {
    check_shader_compile_success(frag);
  } catch (ShaderException& e) {
    log_error("Could not compile fragment shader!");
    glDeleteShader(vert);
    glDeleteShader(frag);
    throw e;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vert);
  glAttachShader(program, frag);
  glLinkProgram(program);

  glDeleteShader(vert);
  glDeleteShader(frag);
  try {
    check_program_link_success(program);
  } catch (ShaderException& e) {
    log_error("Could not link shader program!");
    throw e;
  }

  return program;
}
void Shader::check_shader_compile_success(GLuint shader) {
  constexpr std::size_t buffer_len = 512;
  int success;
  char buffer[buffer_len];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, buffer_len, NULL, buffer);
    throw ShaderException(ShaderException::Type::CompileError, buffer);
  }
}
void Shader::check_program_link_success(GLuint program) {
  constexpr std::size_t buffer_len = 512;
  int success;
  char buffer[buffer_len];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(program, buffer_len, NULL, buffer);
    throw ShaderException(ShaderException::Type::LinkError, buffer);
  }
}

}  // namespace woop