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

struct TriangleMesh
{
  std::vector<Triangle> triangles;
  std::vector<Vertex> vertices;
};
