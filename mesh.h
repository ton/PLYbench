#pragma once

#include <cstdint>
#include <vector>

struct Triangle
{
  std::int32_t a, b, c;
};

struct Vertex
{
  float x, y, z;
};

using Triangles = std::vector<Triangle>;
using Vertices = std::vector<Vertex>;

struct TriangleMesh
{
  Triangles triangles;
  Vertices vertices;
};
