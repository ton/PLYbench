#include "util.h"

#include <stdlib.h>

TriangleMesh createMesh()
{
  constexpr std::int32_t numTriangles = 1000;

  Triangles triangles;
  Vertices vertices(2 + numTriangles);

  for (std::int32_t i = 0; i < numTriangles; ++i)
  {
    triangles.push_back(Triangle{i, i + 1, i + 2});
  }

  return TriangleMesh{std::move(triangles), std::move(vertices)};
}

std::filesystem::path uniquePath()
{
  auto path = std::filesystem::temp_directory_path();
  std::string suffix = "x";
  while (std::filesystem::exists(path / suffix)) { suffix += char(rand() % 26 + 65); }
  return path / suffix;
}

TemporaryFile::TemporaryFile() : filename_{uniquePath()}, stream_{filename_} {}

TemporaryFile::~TemporaryFile()
{
  if (stream_)
  {
    stream_.close();
    if (!keep_)
    {
      std::filesystem::remove(filename_);
    }
  }
}

TemporaryFile::TemporaryFile(TemporaryFile &&x)
    : filename_{std::move(x.filename_)}, keep_{x.keep_}, stream_{std::move(x.stream_)}
{
}

TemporaryFile &TemporaryFile::operator=(TemporaryFile &&x)
{
  filename_ = std::move(x.filename_);
  keep_ = x.keep_;
  stream_ = std::move(x.stream_);
  return *this;
}
