#include "parsers.h"

#include <happly/happly.h>
#include <miniply/miniply.h>
#include <rply/rply.h>
#include <vcglib/wrap/ply/plylib.h>

#include <vcglib/wrap/nanoply/include/nanoply.hpp>

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

  return TriangleMesh{
      Triangles{triangles, triangles + numTriangles}, Vertices{vertices, vertices + numVertices}};
}

std::optional<TriangleMesh> parseNanoPly(const std::string &filename)
{
  nanoply::Info info(filename);

  TriangleMesh mesh;
  mesh.triangles.resize(info.GetFaceCount());
  mesh.vertices.resize(info.GetVertexCount());

  nanoply::ElementDescriptor vertexDescriptor(nanoply::NNP_VERTEX_ELEM);
  nanoply::ElementDescriptor faceDescriptor(nanoply::NNP_FACE_ELEM);
  vertexDescriptor.dataDescriptor.push_back(new nanoply::DataDescriptor<Vertex, 3, float>(
      nanoply::NNP_PXYZ, static_cast<void *>(mesh.vertices.data())));
  faceDescriptor.dataDescriptor.push_back(new nanoply::DataDescriptor<Triangle, 3, std::int32_t>(
      nanoply::NNP_FACE_VERTEX_LIST, static_cast<void *>(mesh.triangles.data())));

  std::vector<nanoply::ElementDescriptor *> meshDescriptor = {&vertexDescriptor, &faceDescriptor};
  OpenModel(info, meshDescriptor);

  for (std::size_t i = 0; i < vertexDescriptor.dataDescriptor.size(); i++)
  {
    delete vertexDescriptor.dataDescriptor[i];
  }
  for (std::size_t i = 0; i < faceDescriptor.dataDescriptor.size(); i++)
  {
    delete faceDescriptor.dataDescriptor[i];
  }

  return mesh;
}

std::optional<TriangleMesh> parsePlyLib(const std::string &filename)
{
  using namespace vcg::ply;

  PlyFile pf;
  pf.Open(filename.c_str(), PlyFile::MODE_READ);
  pf.AddToRead("vertex", "x", T_FLOAT, T_FLOAT, offsetof(Vertex, x), 0, 0, 0, 0, 0);
  pf.AddToRead("vertex", "y", T_FLOAT, T_FLOAT, offsetof(Vertex, y), 0, 0, 0, 0, 0);
  pf.AddToRead("vertex", "z", T_FLOAT, T_FLOAT, offsetof(Vertex, z), 0, 0, 0, 0, 0);
  pf.AddToRead(
      "face", "vertex_indices", T_INT, T_INT, offsetof(Triangle, a), 1, 0, T_UCHAR, T_UCHAR, 0);

  TriangleMesh mesh;

  for (std::size_t i = 0; i < pf.elements.size(); i++)
  {
    const std::size_t n = pf.ElemNumber(i);

    if (!strcmp(pf.ElemName(i), "vertex"))
    {
      pf.SetCurElement(i);

      mesh.vertices.resize(n);
      for (std::size_t j = 0; j < n; ++j) { pf.Read(static_cast<void *>(&mesh.vertices[j])); }
    }
    else if (!strcmp(pf.ElemName(i), "face"))
    {
      pf.SetCurElement(i);

      mesh.triangles.resize(n);
      for (std::size_t j = 0; j < n; ++j) { pf.Read(static_cast<void *>(&mesh.triangles[j])); }
    }
  }

  pf.Destroy();

  return mesh;
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
    else { plyIn.skipElement(); }
  }

  return TriangleMesh{std::move(triangles), std::move(vertices)};
}

std::optional<TriangleMesh> parseRPly(const std::string &filename)
{
  p_ply ply = ply_open(filename.c_str(), nullptr, 0, nullptr);

  if (!ply) { return std::nullopt; }
  if (!ply_read_header(ply)) { return std::nullopt; }

  TriangleMesh mesh;

  p_ply_element element = nullptr;
  while ((element = ply_get_next_element(ply, element)))
  {
    const char *name;
    long n;
    ply_get_element_info(element, &name, &n);
    if (!strcmp(name, "vertex")) { mesh.vertices.reserve(n); }
    if (!strcmp(name, "face")) { mesh.triangles.reserve(n); }
  }

  auto readVertex = [](p_ply_argument argument)
  {
    void *pdata;
    long idata;
    ply_get_argument_user_data(argument, &pdata, &idata);

    TriangleMesh *mesh = static_cast<TriangleMesh *>(pdata);
    const int val_idx = idata;

    const float value = static_cast<float>(ply_get_argument_value(argument));

    switch (val_idx)
    {
      case 0:
        mesh->vertices.push_back(Vertex{value, 0, 0});
        break;
      case 1:
        mesh->vertices.back().y = value;
        break;
      case 2:
        mesh->vertices.back().z = value;
        break;
    }

    return 1;
  };

  auto readTriangle = [](p_ply_argument argument)
  {
    void *pdata;
    ply_get_argument_user_data(argument, &pdata, NULL);

    TriangleMesh *mesh = static_cast<TriangleMesh *>(pdata);

    long length, val_idx;
    ply_get_argument_property(argument, nullptr, &length, &val_idx);

    const std::int32_t value = static_cast<std::int32_t>(ply_get_argument_value(argument));

    switch (val_idx)
    {
      case 0:
        mesh->triangles.push_back(Triangle{value, 0, 0});
        break;
      case 1:
        mesh->triangles.back().b = value;
        break;
      case 2:
        mesh->triangles.back().c = value;
        break;
      default:
        break;
    }

    return 1;
  };

  ply_set_read_cb(ply, "vertex", "x", readVertex, &mesh, 0);
  ply_set_read_cb(ply, "vertex", "y", readVertex, &mesh, 1);
  ply_set_read_cb(ply, "vertex", "z", readVertex, &mesh, 2);
  ply_set_read_cb(ply, "face", "vertex_indices", readTriangle, &mesh, 0);

  if (!ply_read(ply)) { return std::nullopt; }

  ply_close(ply);

  return mesh;
}
