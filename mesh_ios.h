#pragma once

#include "mesh.h"

#include <ostream>

inline std::ostream &operator<<(std::ostream &os, const Triangle &t)
{
  return os << t.a << ", " << t.b << ", " << t.c;
}

inline std::ostream &operator<<(std::ostream &os, const Vertex &v)
{
  return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}
