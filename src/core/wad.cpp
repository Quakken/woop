/**
 * @file wad.cpp
 * @authors quak
 * @brief Defines members of the Wad class.
 */

#include "wad.hpp"
#include <fstream> /* std::ifstream */

namespace woop {
Wad::Wad() : type(WadType::Unloaded) {}
Wad::Wad(const std::filesystem::path& path) {
  open(path);
}

void Wad::open(const std::filesystem::path& path) {
  // Release all data from previous openings
  close();
  // Open file
  std::ifstream file(path, std::ifstream::binary);
  if (!file.is_open()) {
    throw WadException(WadException::Type::FileNotFound,
                       "Could not find wad at path " + path.string());
  }
  // Load data
  WadHeader header = parse_header(file);
  get_wad_type(header);
  std::vector<WadEntry> directory = parse_directory(file, header);
  get_lumps_from_directory(file, directory);
  file_loaded = true;
}

void Wad::close() noexcept {
  file_loaded = false;
  type = WadType::Unloaded;
  lumps.clear();
}

WadHeader Wad::parse_header(std::ifstream& file) {
  WadHeader out;
  file.seekg(offsetof(WadHeader, type));
  file.read(out.type, sizeof(out.type));
  file.seekg(offsetof(WadHeader, num_lumps));
  file.read(reinterpret_cast<char*>(&out.num_lumps), sizeof(out.num_lumps));
  file.seekg(offsetof(WadHeader, dir_offset));
  file.read(reinterpret_cast<char*>(&out.dir_offset), sizeof(out.dir_offset));
  return out;
}

void Wad::get_wad_type(const WadHeader& header) {
  std::string type_str(header.type, sizeof(header.type));
  if (type_str == "IWAD")
    type = WadType::Internal;
  else if (type_str == "PWAD")
    type = WadType::Patch;
  else
    throw WadException(WadException::Type::InvalidHeader,
                       "Unknown wad type " + type_str);
}

std::vector<WadEntry> Wad::parse_directory(std::ifstream& file,
                                           const WadHeader& header) {
  if (header.num_lumps < 0)
    throw WadException(WadException::Type::InvalidHeader,
                       "Header contained negative lump count");
  if (header.dir_offset < 0)
    throw WadException(WadException::Type::InvalidHeader,
                       "Header contained negative directory offset");

  // sizeof returns an unsigned value, but streamoff is signed
  std::streamoff signed_entry_size =
      static_cast<std::streamoff>(sizeof(WadEntry));

  std::vector<WadEntry> directory;
  directory.reserve(static_cast<std::size_t>(header.num_lumps));
  for (int i = 0; i < header.num_lumps; ++i) {
    std::streampos entry_offset = header.dir_offset + signed_entry_size * i;
    WadEntry entry;
    // Read offset
    file.seekg(entry_offset +
               static_cast<std::streamoff>(offsetof(WadEntry, offset)));
    file.read(reinterpret_cast<char*>(&entry.offset), sizeof(entry.offset));
    // Read size
    file.seekg(entry_offset +
               static_cast<std::streamoff>(offsetof(WadEntry, size)));
    file.read(reinterpret_cast<char*>(&entry.size), sizeof(entry.size));
    // Read name
    file.seekg(entry_offset +
               static_cast<std::streamoff>(offsetof(WadEntry, name)));
    file.read(reinterpret_cast<char*>(&entry.name), sizeof(entry.name));
    directory.emplace_back(std::move(entry));
  }
  return directory;
}

void Wad::get_lumps_from_directory(std::ifstream& file,
                                   const std::vector<WadEntry>& directory) {
  for (const auto& entry : directory) {
    Lump lump = get_lump_from_entry(file, entry);
    first_occurances.try_emplace(lump.name, lumps.size());
    lumps.emplace_back(std::move(lump));
  }
}

Lump Wad::get_lump_from_entry(std::ifstream& file, const WadEntry& entry) {
  if (entry.offset < 0)
    throw WadException(WadException::Type::InvalidDirectory,
                       "Lump offset is negative");
  if (entry.size < 0)
    throw WadException(WadException::Type::InvalidDirectory,
                       "Lump size is negative");

  Lump out;
  out.name = get_string_from_buff(entry.name, sizeof(entry.name));
  // Virtual entries don't have any data
  if (entry.size == 0)
    return out;

  std::size_t u_lump_size = static_cast<std::size_t>(entry.size);
  out.data.resize(u_lump_size);
  file.seekg(entry.offset);
  file.read(reinterpret_cast<char*>(&out.data.front()), entry.size);
  return out;
}

std::string Wad::get_string_from_buff(const char* buffer,
                                      std::size_t size) noexcept {
  while (size > 0 && !buffer[size - 1])
    --size;
  if (size == 0 && !buffer[size])
    return std::string{};
  return std::string(buffer, size);
}
}  // namespace woop