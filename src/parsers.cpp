#include "parsers.h"

#include <happly/happly.h>
#include <miniply/miniply.h>

// clang-format off
#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_INCLUDE_LIBC_HEADERS
#define MSH_ARGPARSE_IMPLEMENTATION
#define MSH_PLY_INCLUDE_LIBC_HEADERS
#define MSH_PLY_IMPLEMENTATION
#include <assert.h>
#include <msh/msh_argparse.h>
#include <msh/msh_ply.h>
#include <msh/msh_std.h>
// clang-format on

#include <plywoot/plywoot.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

std::optional<TriangleMesh> parseHapply(const std::string &filename)
{
  // Construct the data object by reading from file
  happly::PLYData plyIn(filename);

  if (!plyIn.hasElement("vertex")) { return std::nullopt; }

  if (!plyIn.hasElement("face")) { return std::nullopt; }

  // Convert hapPLY vertices to mesh vertices.
  auto &vertexElement = plyIn.getElement("vertex");
  std::vector<float> x = vertexElement.getProperty<float>("x");
  std::vector<float> y = vertexElement.getProperty<float>("y");
  std::vector<float> z = vertexElement.getProperty<float>("z");
  if (x.size() != y.size() || x.size() != z.size()) { return std::nullopt; }

  std::vector<Vertex> vertices;
  vertices.reserve(x.size());
  for (size_t i = 0; i < x.size(); ++i) { vertices.push_back(Vertex{x[i], y[i], z[i]}); }

  // Convert hapPLY triangles to mesh triangles.
  std::vector<std::vector<std::int32_t>> happlyTriangles = plyIn.getFaceIndices<std::int32_t>();
  std::vector<Triangle> triangles;
  triangles.reserve(happlyTriangles.size());
  std::transform(
      happlyTriangles.begin(), happlyTriangles.end(), std::back_inserter(triangles),
      [](const std::vector<std::int32_t> &t) {
        return Triangle{t[0], t[1], t[2]};
      });

  return TriangleMesh{std::move(triangles), std::move(vertices)};
}

std::optional<TriangleMesh> parseMiniply(const std::string &filename)
{
  const int verts_per_face = 3;

  miniply::PLYReader reader{filename.data()};
  if (!reader.valid()) { return std::nullopt; }

  miniply::PLYElement *facesElem =
      reader.get_element(reader.find_element(miniply::kPLYFaceElement));
  if (!facesElem) { return std::nullopt; }

  std::vector<std::uint32_t> listIdxs;
  listIdxs.resize(3);
  facesElem->convert_list_to_fixed_size(
      facesElem->find_property("vertex_indices"), verts_per_face, listIdxs.data());

  TriangleMesh mesh;

  bool gotVerts = false;
  bool gotFaces = false;
  while (reader.has_element() && (!gotVerts || !gotFaces))
  {
    if (!gotVerts && reader.element_is(miniply::kPLYVertexElement))
    {
      if (!reader.load_element()) { return std::nullopt; }

      uint32_t propIdxs[3];
      if (!reader.find_pos(propIdxs)) { break; }
      mesh.vertices.resize(reader.num_rows());
      reader.extract_properties(propIdxs, 3, miniply::PLYPropertyType::Float, mesh.vertices.data());
      gotVerts = true;
    }
    else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement))
    {
      if (!reader.load_element()) { return std::nullopt; }

      mesh.triangles.resize(reader.num_rows());
      reader.extract_properties(
          listIdxs.data(), verts_per_face, miniply::PLYPropertyType::Int, mesh.triangles.data());
      gotFaces = true;
    }
    reader.next_element();
  }

  return mesh;
}

std::optional<TriangleMesh> parseMshPly(const std::string &filename)
{
  const char *vertexProperties[] = {"x", "y", "z"};
  const char *triangleProperties[] = {"vertex_indices"};

  Vertex *vertices;
  Triangle *triangles;

  std::int32_t numVertices = 0;
  std::int32_t numTriangles = 0;

  msh_ply_desc_t vertexDescriptor;
  vertexDescriptor.element_name = const_cast<char *>("vertex");
  vertexDescriptor.property_names = vertexProperties;
  vertexDescriptor.num_properties = 3;
  vertexDescriptor.data_type = MSH_PLY_FLOAT;
  vertexDescriptor.list_type = MSH_PLY_INVALID;
  vertexDescriptor.data = &vertices;
  vertexDescriptor.list_data = nullptr;
  vertexDescriptor.data_count = &numVertices;

  msh_ply_desc_t faceDescriptor;
  faceDescriptor.element_name = const_cast<char *>("face");
  faceDescriptor.property_names = triangleProperties;
  faceDescriptor.num_properties = 1;
  faceDescriptor.data_type = MSH_PLY_INT32;
  faceDescriptor.list_type = MSH_PLY_UINT8;
  faceDescriptor.data = &triangles;
  faceDescriptor.list_data = nullptr;
  faceDescriptor.data_count = &numTriangles;
  faceDescriptor.list_size_hint = 3;

  msh_ply_t *plyFile = msh_ply_open(filename.c_str(), "rb");
  if (!plyFile) { return std::nullopt; }

  msh_ply_add_descriptor(plyFile, &vertexDescriptor);
  msh_ply_add_descriptor(plyFile, &faceDescriptor);
  msh_ply_read(plyFile);
  msh_ply_close(plyFile);

  auto verticesUptr = std::unique_ptr<Vertex, decltype(&free)>(vertices, free);
  auto trianglesUPtr = std::unique_ptr<Triangle, decltype(&free)>(triangles, free);

  return TriangleMesh{Triangles{triangles, triangles + numTriangles}, Vertices{vertices, vertices + numVertices}};
}

std::optional<TriangleMesh> parsePlywoot(const std::string &filename)
{
  std::ifstream ifs{filename};
  if (!ifs) { return std::nullopt; }

  std::vector<Triangle> triangles;
  std::vector<Vertex> vertices;

  plywoot::IStream plyIn{ifs};
  while (plyIn.hasElement())
  {
    const plywoot::PlyElement element{plyIn.element()};
    if (element.name() == "vertex")
    {
      using VertexLayout = plywoot::reflect::Layout<plywoot::reflect::Pack<float, 3>>;
      vertices = plyIn.readElement<Vertex, VertexLayout>();
    }
    else if (element.name() == "face")
    {
      using TriangleLayout = plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;
      triangles = plyIn.readElement<Triangle, TriangleLayout>();
    }
    else
    {
      plyIn.skipElement();
    }
  }

  return TriangleMesh{std::move(triangles), std::move(vertices)};
}
