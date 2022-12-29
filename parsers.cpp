#include "parsers.h"

#include <happly/happly.h>
#include <miniply/miniply.h>

#include <plywoot/plywoot.hpp>

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

std::optional<TriangleMesh> parsePlyWoot(const std::string &filename)
{
  std::ifstream ifs{filename};
  if (!ifs) { return std::nullopt; }

  plywoot::IStream plyIn{ifs};

  plywoot::PlyElement vertexElement;
  bool verticesFound{false};
  std::tie(vertexElement, verticesFound) = plyIn.element("vertex");
  if (!verticesFound) { return std::nullopt; }

  plywoot::PlyElement faceElement;
  bool faceIndicesFound{false};
  std::tie(faceElement, faceIndicesFound) = plyIn.element("face");
  if (!faceIndicesFound) { return std::nullopt; }

  using VertexLayout = plywoot::reflect::Layout<plywoot::reflect::Pack<float, 3>>;
  using TriangleLayout = plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;

  std::vector<Vertex> vertices{plyIn.read<Vertex, VertexLayout>(vertexElement)};
  std::vector<Triangle> triangles{plyIn.read<Triangle, TriangleLayout>(faceElement)};

  return TriangleMesh{std::move(triangles), std::move(vertices)};
}
