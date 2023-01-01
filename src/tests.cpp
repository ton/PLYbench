#include "mesh.h"
#include "mesh_ios.h"
#include "parsers.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <sstream>

std::string meshComparisonInfo(
    const std::optional<TriangleMesh> &maybeX,
    const std::optional<TriangleMesh> &maybeY,
    const std::string &lhsName,
    const std::string &rhsName)
{
  if (bool(maybeX) != bool(maybeY)) { return "one of the meshes is undefined"; }

  if (maybeX && maybeY)
  {
    const TriangleMesh &x = *maybeX;
    const TriangleMesh &y = *maybeY;

    std::ostringstream oss;

    if (x.vertices.size() != y.vertices.size())
    {
      oss << "Number of vertices in " << lhsName << " mesh is " << x.vertices.size()
          << ", whereas the " << rhsName << " mesh contains " << y.vertices.size() << " vertices.";
      return oss.str();
    }

    if (x.triangles.size() != y.triangles.size())
    {
      oss << "Number of triangles in " << lhsName << " mesh is " << x.triangles.size()
          << ", whereas the " << rhsName << " mesh contains " << y.triangles.size()
          << " triangles.";
      return oss.str();
    }

    for (std::size_t i = 0; i < x.vertices.size(); ++i)
    {
      if (x.vertices[i] != y.vertices[i])
      {
        oss << "Vertex " << i << " from the " << lhsName << " mesh has value " << x.vertices[i]
            << ", whereas vertex " << i << " from the " << rhsName
            << " mesh has value: " << y.vertices[i];
        return oss.str();
      }
    }

    for (std::size_t i = 0; i < x.triangles.size(); ++i)
    {
      if (x.triangles[i] != y.triangles[i])
      {
        oss << "Triangle " << i << " from the " << lhsName << " mesh has value " << x.triangles[i]
          << ", whereas triangle " << i << " from the " << rhsName
          << " mesh has value: " << y.triangles[i];
        return oss.str();
      }
    }
  }

  return "No comparison information...";
}

TEST_CASE("Verify parsers against PLYwoot")
{
  auto filename = GENERATE(
      "bun_zipper.ply", "dragon_vrip.ply", "happy_vrip.ply", "lucy.ply", "xyzrgb_dragon.ply",
      "Doom combat scene.ply");

  const auto plywootMesh = parsePlywoot(std::string("models/") + filename);

  SECTION("hapPLY")
  {
    auto mesh = parseHapply(std::string("models/") + filename);

    INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "hapPLY", "PLYwoot"));
    CHECK(mesh == plywootMesh);
  }

  SECTION("MiniPLY")
  {
    auto mesh = parseMiniply(std::string("models/") + filename);

    INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "MiniPLY", "PLYwoot"));
    CHECK(mesh == plywootMesh);
  }

  SECTION("msh_ply")
  {
    auto mesh = parseMshPly(std::string("models/") + filename);

    INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "msh_ply", "PLYwoot"));
    CHECK(mesh == plywootMesh);
  }

  SECTION("nanoply")
  {
    auto mesh = parseNanoPly(std::string("models/") + filename);

    INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "nanoply", "PLYwoot"));
    CHECK(mesh == plywootMesh);
  }
}

int main(int argc, char *argv[]) { return Catch::Session().run(argc, argv); }
