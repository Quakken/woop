/**
 * @file wad.hpp
 * @authors quak
 * @brief Declares the Wad class, which is used to parse DOOM's wad files. Wads
 * are made up of Lumps, which store data about individual parts of the game.
 *
 * See the wiki page for a more detailed overview of the wad file format:
 * https://doomwiki.org/wiki/WAD
 */

#pragma once

#include "exception.hpp" /* woop::Exception */
#include <cstdint>       /* int32_t */
#include <string>        /* std::string */
#include <unordered_map> /* std::unordered_map */
#include <vector>        /* std::vector */
#include <filesystem>    /* std::filesystem::path */

namespace woop {
/**
 * @brief Exception thrown when a wad encounters an error.
 */
class WadException : public Exception {
 public:
  /**
   * @brief Details the cause of the exception.
   */
  enum class Type : uint8_t {
    FileNotFound,
    LumpNotFound,
    InvalidHeader,
    InvalidDirectory,
    BadLumpInterpret,
  };

  WadException(Type type, const std::string_view& what = "WadException")
      : Exception(what), t(type) {}

  /**
   * @brief Returns the type of exception that was thrown
   */
  Type type() const noexcept { return t; }

 private:
  Type t;
};

/**
 * @brief Defines the type of a wad (internal or patch).
 */
enum class WadType : uint8_t {
  Unloaded,
  Internal,
  Patch,
};

/**
 * @brief Stores a wad's header data, which can be used to locate the directory.
 * @note Members are stored exactly as they appear in the wad file's header.
 * This means operators like `sizeof` and `offsetof` also represent the size and
 * position of elements in the wad file.
 */
struct WadHeader {
  char type[4];
  int32_t num_lumps;
  int32_t dir_offset;
};

/**
 * @brief Stores data about a single entry in a wad's directory.
 * @note Members are stored exactly as they appear in the wad file's header.
 * This means operators like `sizeof` and `offsetof` also represent the size and
 * position of elements in the wad file.
 */
struct WadEntry {
  int32_t offset;
  int32_t size;
  char name[8];
};

/**
 * @brief Stores raw lump data from a wad.
 * @note If a lump is virtual, it will not have any associated data.
 */
struct Lump {
  std::string name;
  std::vector<std::byte> data;

  /**
   * @brief Reads the lump's data as though it were a vector of the given type.
   * @note This clones the data into a new array and returns the result.
   */
  template <typename T>
  std::vector<T> get_data_as() const {
    // Make sure sectors can be read safely
    if (data.size() % sizeof(T) != 0)
      throw WadException(WadException::Type::BadLumpInterpret,
                         "Lump could not be interpreted as the given type");

    // Reinterpret lump data as the desired type.
    std::size_t count = data.size() / sizeof(T);
    std::vector<T> out(count);
    std::memcpy(out.data(), data.data(), count * sizeof(T));
    return out;
  }
};

/**
 * @brief Stores data about a wad file, including all lumps it contains.
 */
class Wad {
 public:
  Wad();
  /**
   * @brief Opens a wad file at the given path.
   */
  Wad(const std::filesystem::path& path);

  /**
   * @brief Opens a wad file at the given path.
   */
  void open(const std::filesystem::path& path);
  /**
   * @brief Closes the wad, releasing all data.
   */
  void close() noexcept;

  /**
   * Iterator protocol
   */
  std::vector<Lump>::iterator begin() noexcept { return lumps.begin(); }
  std::vector<Lump>::iterator end() noexcept { return lumps.end(); }

  /**
   * @brief Returns true if the Wad has been read from a file.
   */
  bool is_open() const noexcept { return file_loaded; }
  /**
   * @brief Returns the type of wad that is currently loaded.
   */
  WadType get_type() const noexcept { return type; }
  /**
   * @brief Returns the number of lumps that have been loaded.
   */
  std::size_t get_num_lumps() const noexcept { return lumps.size(); }

  /**
   * @brief Returns a reference to the given lump, or the first lump which comes
   * after the given names.
   * @example `wad.get_lump("COLORMAP")` loads the COLORMAP lump
   * @example `wad.get_lump("E1M1", "THINGS")` loads the first THINGS lump that
   * comes after E1M1
   */
  template <typename... Ts>
  const Lump& get_lump(const std::string& first, Ts&&... next) const {
    if (!is_open())
      throw WadException(WadException::Type::LumpNotFound,
                         "Attempting to load lump from uninitialized wad");
    try {
      std::size_t start = first_occurances.at(first);
      return get_lump_impl(start, first, std::forward<Ts>(next)...);
    } catch (std::exception& except) {
      throw WadException(WadException::Type::LumpNotFound,
                         "Could not find lump " + first);
    }
  }

 private:
  template <typename... Ts>
  const Lump& get_lump_impl(std::size_t offset,
                            const std::string& first,
                            Ts&&... next) const {
    for (std::size_t i = offset; i < lumps.size(); ++i) {
      if (lumps[i].name == first) {
        return get_lump_impl(i, std::forward<Ts>(next)...);
      }
    }
    throw WadException(WadException::Type::LumpNotFound,
                       "Could not find lump " + first);
  }
  const Lump& get_lump_impl(std::size_t offset) const { return lumps[offset]; }

  /**
   * @brief Parses the header of a wad file.
   */
  static WadHeader parse_header(std::ifstream& file);
  /**
   * @brief Determines what type of wad was loaded from a header's type.
   */
  void get_wad_type(const WadHeader& header);
  /**
   * @brief Parses the contents of a wad's directory.
   */
  static std::vector<WadEntry> parse_directory(std::ifstream& file,
                                               const WadHeader& header);
  /**
   * @brief Runs through directory entries, copying data from the wad into lump
   * objects.
   */
  void get_lumps_from_directory(std::ifstream& file,
                                const std::vector<WadEntry>& directory);
  /**
   * @brief Searches a file for the given lump, returning its data.
   */
  static Lump get_lump_from_entry(std::ifstream& file, const WadEntry& entry);
  /**
   * @brief Creates a string from a potentially non-null-terminated buffer.
   */
  static std::string get_string_from_buff(const char* buffer,
                                          std::size_t size) noexcept;

  bool file_loaded;
  WadType type;
  std::vector<Lump> lumps;
  std::unordered_map<std::string, std::size_t> first_occurances;
};
}  // namespace woop