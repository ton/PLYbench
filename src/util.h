#pragma once

#include "mesh.h"

#include <filesystem>
#include <fstream>
#include <string>

TriangleMesh createMesh();

std::filesystem::path uniquePath();

class TemporaryFile
{
public:
  TemporaryFile();
  ~TemporaryFile();

  TemporaryFile(const TemporaryFile &) = delete;
  TemporaryFile &operator=(const TemporaryFile &) = delete;

  TemporaryFile(TemporaryFile &&x);
  TemporaryFile &operator=(TemporaryFile &&x);

  void keep() { keep_ = true; }

  std::filesystem::path filename() const { return filename_; }
  std::ofstream &stream() { return stream_; }

  operator bool() const { return bool(stream_); }

private:
  std::filesystem::path filename_;
  bool keep_{false};
  std::ofstream stream_;
};
