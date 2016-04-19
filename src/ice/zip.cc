#include <ice/zip.h>
#include <ice/date.h>
#include <minizip/unzip.h>
#include <minizip/zip.h>
#ifdef _WIN32
#include <minizip/iowin32.h>
#else
#include <minizip/ioapi.h>
#endif
#include <algorithm>
#include <array>
#include <limits>
#include <stdexcept>

namespace ice {
namespace {

struct file_zip_handle {
public:
  file_zip_handle(void* file, const zip_file& zip_file)
  {
    if (!file) {
      throw std::runtime_error("zip: file error");
    }
    auto dp = date::floor<date::days>(zip_file.tp);
    auto tp = zip_file.tp - dp;

    auto date = date::year_month_day(dp);
    auto time = date::make_time(tp);

    zip_fileinfo info = {};
    info.dosDate = 0;
    info.tmz_date.tm_year = static_cast<int>(date.year());
    info.tmz_date.tm_mon = static_cast<unsigned>(date.month()) - 1;
    info.tmz_date.tm_mday = static_cast<unsigned>(date.day());
    info.tmz_date.tm_hour = time.hours().count();
    info.tmz_date.tm_min = time.minutes().count();
    info.tmz_date.tm_sec = static_cast<uInt>(time.seconds().count());

    auto ret = zipOpenNewFileInZip(file, zip_file.name.c_str(), &info, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (ret != ZIP_OK) {
      throw std::runtime_error("zip: zip file error: " + std::to_string(ret));
    }

    file_ = file;
  }

  ~file_zip_handle()
  {
    if (file_) {
      zipCloseFileInZip(file_);
    }
  }

private:
  void* file_ = nullptr;
};

struct file_unzip_handle {
public:
  file_unzip_handle(void* file)
  {
    if (!file) {
      throw std::runtime_error("unzip: file error");
    }
    if (unzOpenCurrentFile(file) != UNZ_OK) {
      throw std::runtime_error("unzip: zip file error");
    }
    file_ = file;
  }

  ~file_unzip_handle()
  {
    if (file_) {
      unzCloseCurrentFile(file_);
    }
  }

private:
  void* file_ = nullptr;
};

}  // namespace

zip::zip()
{}

zip::zip(const ice::filesystem::path& filename, bool append)
{
  zlib_filefunc_def file_functions = {};
#ifdef _WIN32
  fill_win32_filefunc(&file_functions);
#else
  fill_fopen_filefunc(&file_functions);
#endif
  file_ = zipOpen2(filename.str().c_str(), append ? 1 : 0, nullptr, &file_functions);
  if (!file_) {
    throw std::runtime_error("zip: file error");
  }
}

zip::zip(zip&& other) :
  zip()
{
  std::swap(file_, other.file_);
}

zip& zip::operator=(zip&& other)
{
  std::swap(file_, other.file_);
  return *this;
}

zip::~zip()
{
  if (file_) {
    zipClose(reinterpret_cast<zipFile>(file_), nullptr);
  }
}

void zip::write(const zip_file& file, std::function<std::size_t(std::uint8_t* data, std::size_t size)> handler)
{
  file_zip_handle zip_file(file_, file);
  std::array<std::uint8_t, 8192> buffer;
  std::size_t size = 0;
  do {
    size = handler(buffer.data(), buffer.size());
    if (size > 0) {
      if (zipWriteInFileInZip(file_, buffer.data(), static_cast<uLong>(size)) != ZIP_OK) {
        throw std::runtime_error("zip: zip file data error");
      }
    }
  } while (size > 0);
}

unzip::unzip()
{}

unzip::unzip(const ice::filesystem::path& filename)
{
  zlib_filefunc_def file_functions = {};
#ifdef _WIN32
  fill_win32_filefunc(&file_functions);
#else
  fill_fopen_filefunc(&file_functions);
#endif
  file_ = unzOpen2(filename.str().c_str(), &file_functions);
  if (!file_) {
    throw std::runtime_error("unzip: file error");
  }
}

unzip::unzip(unzip&& other) :
  unzip()
{
  std::swap(file_, other.file_);
}

unzip& unzip::operator=(unzip&& other)
{
  std::swap(file_, other.file_);
  return *this;
}

unzip::~unzip()
{
  if (file_) {
    unzClose(reinterpret_cast<unzFile>(file_));
  }
}

std::size_t unzip::size() const
{
  if (!file_) {
    throw std::runtime_error("unzip: file error");
  }

  unz_global_info info = {};
  if (unzGetGlobalInfo(file_, &info) != UNZ_OK) {
    throw std::runtime_error("unzip: info error");
  }
  return info.number_entry;
}

void unzip::list(std::function<bool(const zip_file& file)> handler) const
{
  if (!file_) {
    throw std::runtime_error("unzip: file error");
  }
  auto ret = unzGoToFirstFile(file_);
  switch (ret) {
  case UNZ_OK: break;
  case UNZ_END_OF_LIST_OF_FILE: return;
  default:
    throw std::runtime_error("unzip: first file error");
    break;
  }
  zip_file file;
  do {
    unz_file_info info = {};
    if (unzGetCurrentFileInfo(file_, &info, nullptr, 0, nullptr, 0, nullptr, 0) != UNZ_OK) {
      throw std::runtime_error("unzip: filename size error");
    }
    file.name.resize(info.size_filename);
    file.size = info.uncompressed_size;
    date::day_point dp = date::year(info.tmu_date.tm_year) / date::month(info.tmu_date.tm_mon + 1) / date::day(info.tmu_date.tm_mday);
    file.tp = dp + std::chrono::hours(info.tmu_date.tm_hour) + std::chrono::minutes(info.tmu_date.tm_min) + std::chrono::seconds(info.tmu_date.tm_sec);
    if (info.size_filename > 0) {
      if (unzGetCurrentFileInfo(file_, &info, &file.name[0], info.size_filename, nullptr, 0, nullptr, 0) != UNZ_OK) {
        throw std::runtime_error("unzip: file info error");
      }
    }
    if (file.size == 0 && !file.name.empty() && file.name.at(file.name.size() - 1) == '/') {
      file.directory = true;
    } else {
      file.directory = false;
    }
    if (!handler(file)) {
      break;
    }
    ret = unzGoToNextFile(file_);
    switch (ret) {
    case UNZ_OK: break;
    case UNZ_END_OF_LIST_OF_FILE: break;
    default:
      throw std::runtime_error("unzip: next file error");
      break;
    }
  } while (ret != UNZ_END_OF_LIST_OF_FILE);
}

void unzip::read(std::function<bool(const std::uint8_t* data, std::size_t size)> handler) const
{
  file_unzip_handle handle(file_);
  std::array<std::uint8_t, 8192> buffer;
  int size = 0;
  do {
    size = unzReadCurrentFile(file_, buffer.data(), static_cast<uLong>(buffer.size()));
    if (size > 0) {
      if (!handler(buffer.data(), size)) {
        break;
      }
    }
  } while (size > 0);
}

}  // namespace ice