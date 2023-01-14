#pragma once

#include <cstdint>
#include <vector>

struct Triangle
{
  std::int32_t a, b, c;

  friend bool operator==(const Triangle &x, const Triangle &y)
  {
    return x.a == y.a && x.b == y.b && x.c == y.c;
  }
  friend bool operator!=(const Triangle &x, const Triangle &y) { return !(x == y); }
};

struct Vertex
{
  float x, y, z;

  friend bool operator==(const Vertex &v, const Vertex &w) { return v.x == w.x && v.y == w.y && v.z == w.z; }
  friend bool operator!=(const Vertex &v, const Vertex &w) { return !(v == w); }
};

using Triangles = std::vector<Triangle>;
using Vertices = std::vector<Vertex>;

struct TriangleMesh
{
  Triangles triangles;
  Vertices vertices;

  friend bool operator==(const TriangleMesh &x, const TriangleMesh &y)
  {
    return x.triangles == y.triangles && x.vertices == y.vertices;
  }
  friend bool operator!=(const TriangleMesh &x, const TriangleMesh &y) { return !(x == y); }
};
