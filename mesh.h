#pragma once

#include <cstdint>
#include <vector>

struct Vertex
{
  float x, y, z;
};

struct Triangle
{
  std::int32_t a, b, c;
};

using Triangles = std::vector<Triangle>;
using Vertices = std::vector<Vertex>;

struct TriangleMesh
{
  Triangles triangles;
  Vertices vertices;
};
