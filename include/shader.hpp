#pragma once

#include "exception.hpp" /* woop::Exception */
#include "glad/glad.h"   /* OpenGL */
#include "glm/vec2.hpp"  /* glm::vec2 */
#include "glm/vec3.hpp"  /* glm::vec3 */
#include "glm/vec4.hpp"  /* glm::vec4 */
#include <string>        /* std::string */
#include <filesystem>    /* std::filesystem::Path */

namespace woop {
class ShaderException : public Exception {
 public:
  enum class Type {
    CompileError,
    LinkError,
    InvalidPath,
    InvalidSource,
    InvalidUse,
  };
  ShaderException(Type type, const std::string_view& what)
      : Exception(what), t(type) {}
  Type type() const noexcept { return t; }

 private:
  Type t;
};

/**
 * @brief Represents an OpenGL shader.
 */
class Shader {
 public:
  Shader(const std::string& vert_src, const std::string& frag_src);
  Shader(const Shader& other) = delete;
  Shader(Shader&& other);
  ~Shader();

  Shader& operator=(const Shader& other) = delete;
  Shader& operator=(Shader&& other);

  static Shader from_file(const std::filesystem::path& vert_path,
                          const std::filesystem::path& frag_path);

  void set_uniform(const std::string& name, bool v) const;
  void set_uniform(const std::string& name, int v) const;
  void set_uniform(const std::string& name, unsigned v) const;
  void set_uniform(const std::string& name, float v) const;
  void set_uniform(const std::string& name, const glm::vec2& vec) const;
  void set_uniform(const std::string& name, const glm::vec3& vec) const;
  void set_uniform(const std::string& name, const glm::vec4& vec) const;

  void use() const;

  bool is_valid() const noexcept { return !invalid; }

 private:
  static GLuint compile(const std::string& vert_src,
                        const std::string& frag_src);
  static void check_shader_compile_success(GLuint shader);
  static void check_program_link_success(GLuint program);

  bool invalid;
  GLuint program;
};
}  // namespace woop