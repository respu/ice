#pragma once
#include <ice/filesystem/path.h>
#include <chrono>
#include <functional>
#include <cstdint>

namespace ice {

struct zip_file {
  std::string name;
  std::size_t size;
  std::chrono::system_clock::time_point tp;
  bool directory;
};

class zip {
public:
  zip();
  zip(const ice::filesystem::path& filename, bool append);

  zip(zip&& other);
  zip(const zip& other) = delete;

  zip& operator=(zip&& other);
  zip& operator=(const zip& other) = delete;

  ~zip();

  void write(const zip_file& file, std::function<std::size_t(std::uint8_t* data, std::size_t size)> handler);

private:
  void* file_ = nullptr;
};

class unzip {
public:
  unzip();
  unzip(const ice::filesystem::path& filename);

  unzip(unzip&& other);
  unzip(const unzip& other) = delete;

  unzip& operator=(unzip&& other);
  unzip& operator=(const unzip& other) = delete;

  ~unzip();

  // Returns the number of files in the archive.
  std::size_t size() const;

  // Calls the handler with the current file info for every file.
  void list(std::function<bool(const zip_file& file)> handler) const;

  // Calls the handler with the current file data.
  void read(std::function<bool(const std::uint8_t* data, std::size_t size)> handler) const;

private:
  void* file_ = nullptr;
};

}  // namespace ice

//void test()
//{
//  auto path = application::path().parent_path();
//
//  auto src = path / "html";
//  auto dst = path / "html.zip";
//
//  ice::zip zip(dst, false);
//  auto prefix = path.str().size() + 1;
//  std::function<bool(const ice::filesystem::path& entry)> handler = [&](const ice::filesystem::path& entry)
//  {
//    if (entry.is_directory()) {
//      entry.list(handler);
//    } else {
//      ice::zip_file zip_file;
//      zip_file.size = 0;
//      zip_file.tp = entry.modified();
//      zip_file.name = entry.str().substr(prefix);
//      std::replace(zip_file.name.begin(), zip_file.name.end(), '\\', '/');
//
//      ice::log::debug() << zip_file.name;
//      
//      std::ifstream is(entry.wstr(), std::ios::binary);
//      if (!is) {
//        throw std::runtime_error("Could not open file: " + entry.str());
//      }
//      zip.write(zip_file, [&is](std::uint8_t* data, std::size_t size)
//      {
//        is.read(reinterpret_cast<char*>(data), size);
//        return static_cast<std::size_t>(is.gcount());
//      });
//    }
//    return true;  // continue
//  };
//
//  src.list(handler);
//}

//void test()
//{
//  auto path = application::path().parent_path();
//
//  auto src = path / "html.zip";
//
//  ice::unzip unzip(src);
//  ice::log::info() << src.str() << " (" << unzip.size() << " files)";
//  unzip.list([&](const ice::zip_file& file)
//  {
//    auto dp = std::chrono::floor<ice::date::days>(file.tp);
//    auto tp = file.tp - dp;
//    
//    auto date = ice::date::year_month_day(dp);
//    auto time = ice::date::make_time(tp);
//
//    if (file.name == "html/index.html") {
//      ice::log::notice() << date << " " << time << " " << "\"" << file.name << "\" (" << file.size << " bytes)";
//      unzip.read([](const std::uint8_t* data, std::size_t size)
//      {
//        ice::log::debug() << std::string(reinterpret_cast<const char*>(data), size);
//        return true;
//      });
//    }
//    return true;  // continue
//  });
//}