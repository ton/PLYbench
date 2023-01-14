#pragma once

#include "mesh.h"
#include "util.h"

#include <string>

enum class Format { Ascii, BinaryLittleEndian, BinaryBigEndian };

inline std::string formatToString(Format format)
{
  switch (format)
  {
    case Format::Ascii:
      return "ASCII";
    case Format::BinaryBigEndian:
      return "Binary big endian";
    case Format::BinaryLittleEndian:
      return "Binary little endian";
  }

  return {};
}

TemporaryFile writeHapply(const TriangleMesh &mesh, Format format);
TemporaryFile writeMshPly(const TriangleMesh &mesh, Format format);
TemporaryFile writeNanoPly(const TriangleMesh &mesh, Format format);
TemporaryFile writePlywoot(const TriangleMesh &mesh, Format format);
TemporaryFile writeRPly(const TriangleMesh &mesh, Format format);
TemporaryFile writeTinyply(const TriangleMesh &mesh, Format format);
