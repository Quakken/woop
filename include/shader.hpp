#pragma once

#include "exception.hpp" /* woop::Exception */
#include "glad/glad.h"   /* OpenGL */
#include "glm/fwd.hpp"   /* glm::vec2, glm::vec3, glm::vec4 */
#include <string>        /* std::string */
#include <filesystem>    /* std::filesystem::Path */

namespace woop {
class ShaderException : public Exception {
 public:
  enum class Type {
    CompileError,
    LinkError,
    InvalidPath,
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
  ~Shader();

  Shader& operator=(const Shader& other) = delete;

  static Shader from_file(const std::filesystem::path& vert_path,
                          const std::filesystem::path& frag_path);

  void set_uniform(const std::string& name, bool v);
  void set_uniform(const std::string& name, int v);
  void set_uniform(const std::string& name, float v);
  void set_uniform(const std::string& name, const glm::vec2& vec);
  void set_uniform(const std::string& name, const glm::vec3& vec);
  void set_uniform(const std::string& name, const glm::vec4& vec);

  void use();

 private:
  static GLuint compile(const std::string& vert_src,
                        const std::string& frag_src);

  GLuint program_id;
};
}  // namespace woop