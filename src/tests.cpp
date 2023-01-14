#include "mesh.h"
#include "mesh_ios.h"
#include "parsers.h"
#include "util.h"
#include "writers.h"

#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

namespace {
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
      oss << "Number of vertices in " << lhsName << " mesh is " << x.vertices.size() << ", whereas the "
          << rhsName << " mesh contains " << y.vertices.size() << " vertices.";
      return oss.str();
    }

    if (x.triangles.size() != y.triangles.size())
    {
      oss << "Number of triangles in " << lhsName << " mesh is " << x.triangles.size() << ", whereas the "
          << rhsName << " mesh contains " << y.triangles.size() << " triangles.";
      return oss.str();
    }

    for (std::size_t i = 0; i < x.vertices.size(); ++i)
    {
      if (x.vertices[i] != y.vertices[i])
      {
        oss << "Vertex " << i << " from the " << lhsName << " mesh has value " << x.vertices[i]
            << ", whereas vertex " << i << " from the " << rhsName << " mesh has value: " << y.vertices[i];
        return oss.str();
      }
    }

    for (std::size_t i = 0; i < x.triangles.size(); ++i)
    {
      if (x.triangles[i] != y.triangles[i])
      {
        oss << "Triangle " << i << " from the " << lhsName << " mesh has value " << x.triangles[i]
            << ", whereas triangle " << i << " from the " << rhsName << " mesh has value: " << y.triangles[i];
        return oss.str();
      }
    }
  }

  return "No comparison information...";
}
}

TEST_CASE("Verify parsers against PLYwoot")
{
  auto filename = GENERATE(
      "bun_zipper.ply", "dragon_remeshed.ply", "dragon_vrip.ply", "happy_vrip.ply", "lucy.ply",
      "xyzrgb_dragon.ply", "Doom combat scene.ply");

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
    if (std::string(filename) != "dragon_remeshed.ply")
    {
      auto mesh = parseNanoPly(std::string("models/") + filename);

      INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "nanoply", "PLYwoot"));
      CHECK(mesh == plywootMesh);
    }
  }

  SECTION("plylib")
  {
    auto mesh = parsePlyLib(std::string("models/") + filename);

    INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "plylib", "PLYwoot"));
    CHECK(mesh == plywootMesh);
  }

  SECTION("RPly")
  {
    auto mesh = parseRPly(std::string("models/") + filename);

    INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "RPly", "PLYwoot"));
    CHECK(mesh == plywootMesh);
  }
}

// Note; tinyply 2.3 is broken for ASCII PLY files (see:
// https://github.com/ddiakopoulos/tinyply/issues/59).
TEST_CASE("Verify tinyply against PLYwoot")
{
  auto filename = GENERATE("dragon_remeshed.ply", "lucy.ply", "xyzrgb_dragon.ply", "Doom combat scene.ply");

  const auto plywootMesh = parsePlywoot(std::string("models/") + filename);

  auto mesh = parseTinyply(std::string("models/") + filename);

  INFO(std::string{filename} + ": " + meshComparisonInfo(mesh, plywootMesh, "tinyply", "PLYwoot"));
  CHECK(mesh == plywootMesh);
}

TEST_CASE("Test functionality of various writer libraries")
{
  auto format = GENERATE(Format::Ascii, Format::BinaryLittleEndian);

  const TriangleMesh mesh = createMesh(1000);

  SECTION(std::string{"hapPLY ("} + formatToString(format) + ')')
  {
    TemporaryFile tf = writeHapply(mesh, format);
    REQUIRE(bool(tf));
    tf.stream().flush();

    const std::optional<TriangleMesh> maybeMesh = parsePlywoot(tf.filename());
    REQUIRE(maybeMesh.has_value());
    CHECK(mesh == *maybeMesh);
  }

  SECTION(std::string{"msh_ply ("} + formatToString(format) + ')')
  {
    TemporaryFile tf = writeMshPly(mesh, format);
    REQUIRE(bool(tf));
    tf.stream().flush();

    const std::optional<TriangleMesh> maybeMesh = parsePlywoot(tf.filename());
    REQUIRE(maybeMesh.has_value());
    CHECK(mesh == *maybeMesh);
  }

  SECTION(std::string{"nanoply ("} + formatToString(format) + ')')
  {
    TemporaryFile tf = writeNanoPly(mesh, format);
    REQUIRE(bool(tf));
    tf.stream().flush();

    const std::optional<TriangleMesh> maybeMesh = parsePlywoot(tf.filename());
    REQUIRE(maybeMesh.has_value());
    CHECK(mesh == *maybeMesh);
  }

  SECTION(std::string{"PLYwoot ("} + formatToString(format) + ')')
  {
    TemporaryFile tf = writePlywoot(mesh, format);
    REQUIRE(bool(tf));
    tf.stream().flush();

    const std::optional<TriangleMesh> maybeMesh = parsePlywoot(tf.filename());
    REQUIRE(maybeMesh.has_value());
    CHECK(mesh == *maybeMesh);
  }

  SECTION(std::string{"RPly ("} + formatToString(format) + ')')
  {
    TemporaryFile tf = writeRPly(mesh, format);
    REQUIRE(bool(tf));
    tf.stream().flush();

    const std::optional<TriangleMesh> maybeMesh = parsePlywoot(tf.filename());
    REQUIRE(maybeMesh.has_value());
    CHECK(mesh == *maybeMesh);
  }

  SECTION(std::string{"tinyply ("} + formatToString(format) + ')')
  {
    TemporaryFile tf = writeTinyply(mesh, format);
    REQUIRE(bool(tf));
    tf.stream().flush();

    const std::optional<TriangleMesh> maybeMesh = parsePlywoot(tf.filename());
    REQUIRE(maybeMesh.has_value());
    CHECK(mesh == *maybeMesh);
  }
}

int main(int argc, char *argv[]) { return Catch::Session().run(argc, argv); }
