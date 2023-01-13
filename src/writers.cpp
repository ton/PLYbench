#include "writers.h"

#include "msh_ply.h"
#include "util.h"

#include <plywoot/plywoot.hpp>

#include <happly/happly.h>
#include <miniply/miniply.h>
#include <rply/rply.h>
#include <tinyply/source/tinyply.h>
#include <vcglib/wrap/nanoply/include/nanoply.hpp>
#include <vcglib/wrap/ply/plylib.h>

#include <filesystem>
#include <fstream>
#include <string>

TemporaryFile writeHapply(const TriangleMesh &mesh, Format format)
{
  happly::PLYData plyOut;

  plyOut.addElement("vertex", mesh.vertices.size());
  plyOut.addElement("face", mesh.triangles.size());

  std::vector<float> x;
  std::vector<float> y;
  std::vector<float> z;
  x.reserve(mesh.vertices.size());
  y.reserve(mesh.vertices.size());
  z.reserve(mesh.vertices.size());

  for (const Vertex &v : mesh.vertices)
  {
    x.push_back(v.x);
    y.push_back(v.y);
    z.push_back(v.z);
  }

  plyOut.getElement("vertex").addProperty<float>("x", x);
  plyOut.getElement("vertex").addProperty<float>("y", y);
  plyOut.getElement("vertex").addProperty<float>("z", z);

  std::vector<std::vector<int>> indices;
  indices.reserve(mesh.triangles.size());
  for (const Triangle &t : mesh.triangles)
  {
    std::vector<int> triangle = {t.a, t.b, t.c};
    indices.push_back(std::move(triangle));
  }

  plyOut.getElement("face").addListProperty<int>("vertex_indices", indices);

  TemporaryFile tf;
  plyOut.write(
      tf.stream(),
      format == Format::Ascii ? happly::DataFormat::ASCII : happly::DataFormat::Binary);
  return tf;
}

TemporaryFile writeMshPly(const TriangleMesh &mesh, Format format)
{
  const char *vertexProperties[] = {"x", "y", "z"};
  const char *triangleProperties[] = {"vertex_indices"};

  Vertex *vertices = const_cast<Vertex *>(mesh.vertices.data());
  Triangle *triangles = const_cast<Triangle *>(mesh.triangles.data());
  std::int32_t numTriangles = mesh.triangles.size();
  std::int32_t numVertices = mesh.vertices.size();

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

  TemporaryFile tf;

  msh_ply_t *pf = msh_ply_open(tf.filename().c_str(), format == Format::Ascii ? "w" : "wb");
  if (pf)
  {
    msh_ply_add_descriptor(pf, &vertexDescriptor);
    msh_ply_add_descriptor(pf, &faceDescriptor);
    msh_ply_write(pf);
  }
  msh_ply_close(pf);

  return tf;
}

TemporaryFile writeNanoPly(const TriangleMesh &mesh, Format format)
{
  std::vector<nanoply::PlyProperty> vertexProperty;
  vertexProperty.push_back(nanoply::PlyProperty(nanoply::NNP_FLOAT32, nanoply::NNP_PXYZ));

  std::vector<nanoply::PlyProperty> faceProperty;
  faceProperty.push_back(
      nanoply::PlyProperty(nanoply::NNP_LIST_UINT8_UINT32, nanoply::NNP_FACE_VERTEX_LIST));

  nanoply::PlyElement vertexElement(nanoply::NNP_VERTEX_ELEM, vertexProperty, mesh.vertices.size());
  nanoply::PlyElement faceElement(nanoply::NNP_FACE_ELEM, faceProperty, mesh.triangles.size());

  // Create the vertex properties descriptor (what ply property and where the data is stored)
  nanoply::ElementDescriptor vertex(nanoply::NNP_VERTEX_ELEM);
  if (!mesh.vertices.empty())
  {
    vertex.dataDescriptor.push_back(new nanoply::DataDescriptor<Vertex, 3, float>(
        nanoply::NNP_PXYZ, const_cast<Vertex *>(mesh.vertices.data())));
  }

  // Create the face properties descriptor (what ply property and where the data is stored)
  nanoply::ElementDescriptor face(nanoply::NNP_FACE_ELEM);
  if (!mesh.triangles.empty())
  {
    face.dataDescriptor.push_back(new nanoply::DataDescriptor<Triangle, 3, std::int32_t>(
        nanoply::NNP_FACE_VERTEX_LIST, const_cast<Triangle *>(mesh.triangles.data())));
  }

  std::vector<nanoply::ElementDescriptor *> meshDescriptors = {&vertex, &face};

  TemporaryFile tf;

  nanoply::Info info;
  info.filename = tf.filename();
  info.binary = format != Format::Ascii;
  info.AddPlyElement(vertexElement);
  info.AddPlyElement(faceElement);
  nanoply::SaveModel(info.filename, meshDescriptors, info);

  for (std::size_t i = 0; i < vertex.dataDescriptor.size(); i++)
  {
    delete vertex.dataDescriptor[i];
  }
  for (std::size_t i = 0; i < face.dataDescriptor.size(); i++) { delete face.dataDescriptor[i]; }

  return tf;
}

TemporaryFile writePlywoot(const TriangleMesh &mesh, Format format)
{
  plywoot::OStream plyos{
      format == Format::Ascii ? plywoot::PlyFormat::Ascii : plywoot::PlyFormat::BinaryLittleEndian};

  const plywoot::PlyProperty x{"x", plywoot::PlyDataType::Float};
  const plywoot::PlyProperty y{"y", plywoot::PlyDataType::Float};
  const plywoot::PlyProperty z{"z", plywoot::PlyDataType::Float};
  const plywoot::PlyElement vertexElement{"vertex", mesh.vertices.size(), {x, y, z}};

  const plywoot::PlyProperty faceIndices{
      "vertex_indices", plywoot::PlyDataType::Int, plywoot::PlyDataType::UChar};
  const plywoot::PlyElement faceElement{"face", mesh.triangles.size(), {faceIndices}};

  using TriangleLayout = plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;
  using VertexLayout = plywoot::reflect::Layout<plywoot::reflect::Pack<float, 3>>;

  std::vector<Triangle> triangles;
  plyos.add(vertexElement, VertexLayout{mesh.vertices});
  plyos.add(faceElement, TriangleLayout{mesh.triangles});

  TemporaryFile tf;
  plyos.write(tf.stream());
  return tf;
}

TemporaryFile writeRPly(const TriangleMesh &mesh, Format format)
{
  TemporaryFile tf;

  const e_ply_storage_mode mode = format == Format::Ascii ? PLY_ASCII : PLY_LITTLE_ENDIAN;
  p_ply ply = ply_create(tf.filename().c_str(), mode, NULL, 0, NULL);

  if (ply)
  {
    ply_add_element(ply, "vertex", mesh.vertices.size());
    ply_add_scalar_property(ply, "x", PLY_FLOAT);
    ply_add_scalar_property(ply, "y", PLY_FLOAT);
    ply_add_scalar_property(ply, "z", PLY_FLOAT);

    ply_add_element(ply, "face", mesh.triangles.size());
    ply_add_list_property(ply, "vertex_indices", PLY_UINT8, PLY_INT);

    ply_write_header(ply);

    for (const Vertex &v : mesh.vertices)
    {
      ply_write(ply, v.x);
      ply_write(ply, v.y);
      ply_write(ply, v.z);
    }

    for (const Triangle &t : mesh.triangles)
    {
      ply_write(ply, 3);
      ply_write(ply, t.a);
      ply_write(ply, t.b);
      ply_write(ply, t.c);
    }

    ply_close(ply);
  }

  return tf;
}

TemporaryFile writeTinyply(const TriangleMesh &mesh, Format format)
{
  TemporaryFile tf;

  tinyply::PlyFile outFile;
  outFile.add_properties_to_element(
      "vertex", {"x", "y", "z"}, tinyply::Type::FLOAT32, mesh.vertices.size(),
      reinterpret_cast<uint8_t *>(const_cast<Vertex *>(mesh.vertices.data())),
      tinyply::Type::INVALID, 0);
  outFile.add_properties_to_element(
      "face", {"vertex_indices"}, tinyply::Type::UINT32, mesh.triangles.size(),
      reinterpret_cast<uint8_t *>(const_cast<Triangle *>(mesh.triangles.data())),
      tinyply::Type::UINT8, 3);
  outFile.write(tf.stream(), format != Format::Ascii);

  return tf;
}
