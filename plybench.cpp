#include "mesh.h"
#include "submodules/happly/happly.h"
#include "submodules/miniply/miniply.h"

#include <plywoot/plywoot.hpp>

#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <vector>

static void BM_ParseHapply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    // Construct the data object by reading from file
    happly::PLYData plyIn(filename);

    if (!plyIn.hasElement("vertex"))
    {
      state.SkipWithError("could not load vertex data; vertex element not found in input data");
    }

    if (!plyIn.hasElement("face"))
    {
      state.SkipWithError("could not load triangle data; face element not found in input data");
    }

    // Convert hapPLY vertices to mesh vertices.
    auto& vertexElement = plyIn.getElement("vertex");
    std::vector<float> x = vertexElement.getProperty<float>("x");
    std::vector<float> y = vertexElement.getProperty<float>("y");
    std::vector<float> z = vertexElement.getProperty<float>("z");
    if (x.size() != y.size() || x.size() != z.size())
    {
      state.SkipWithError("could not load vertex data; invalid vertex data found in input data");
    }

    std::vector<Vertex> vertices;
    vertices.reserve(x.size());
    for (size_t i = 0; i < x.size(); ++i)
    {
      vertices.push_back(Vertex{x[i], y[i], z[i]});
    }

    // Convert hapPLY triangles to mesh triangles.
    std::vector<std::vector<std::int32_t>> happlyTriangles = plyIn.getFaceIndices<std::int32_t>();
    std::vector<Triangle> triangles;
    triangles.reserve(happlyTriangles.size());
    std::transform(
        happlyTriangles.begin(), happlyTriangles.end(), std::back_inserter(triangles),
        [](const std::vector<std::int32_t> &t) {
          return Triangle{t[0], t[1], t[2]};
        });

    // Finally, construct the mesh.
    const TriangleMesh mesh{std::move(triangles), std::move(vertices)};
    benchmark::DoNotOptimize(mesh);
  }
}

static void BM_ParsePlywoot(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    std::ifstream ifs{filename};
    if (!ifs) { state.SkipWithError((std::string{"could not open: "} + filename).data()); }

    plywoot::IStream plyIn{ifs};

    plywoot::PlyElement vertexElement;
    bool verticesFound{false};
    std::tie(vertexElement, verticesFound) = plyIn.element("vertex");
    if (!verticesFound)
    {
      state.SkipWithError("could not load vertex data; vertex element not found in input data");
    }

    plywoot::PlyElement faceElement;
    bool faceIndicesFound{false};
    std::tie(faceElement, faceIndicesFound) = plyIn.element("face");
    if (!faceIndicesFound)
    {
      state.SkipWithError("could not load triangle data; face element not found in input data");
    }

    using VertexLayout = plywoot::reflect::Layout<plywoot::reflect::Pack<float, 3>>;
    using TriangleLayout = plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;

    std::vector<Vertex> vertices{plyIn.read<Vertex, VertexLayout>(vertexElement)};
    std::vector<Triangle> triangles{plyIn.read<Triangle, TriangleLayout>(faceElement)};

    const TriangleMesh mesh{std::move(triangles), std::move(vertices)};
    benchmark::DoNotOptimize(mesh);
  }
}

static void BM_ParseMiniply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    const int verts_per_face = 3;

    miniply::PLYReader reader{filename.data()};
    if (!reader.valid())
    {
      state.SkipWithError((std::string{"could not open: "} + filename).data());
    }

    miniply::PLYElement *facesElem =
        reader.get_element(reader.find_element(miniply::kPLYFaceElement));
    if (!facesElem)
    {
      state.SkipWithError("could not load triangle data; face element not found in input data");
    }

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
        if (!reader.load_element())
        {
          state.SkipWithError("could not load vertex data; vertex element not found in input data");
        }

        uint32_t propIdxs[3];
        if (!reader.find_pos(propIdxs)) { break; }
        mesh.vertices.resize(reader.num_rows());
        reader.extract_properties(
            propIdxs, 3, miniply::PLYPropertyType::Float, mesh.vertices.data());
        gotVerts = true;
      }
      else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement))
      {
        if (!reader.load_element())
        {
          state.SkipWithError("could not load triangle data; could not load face element");
        }

        mesh.triangles.resize(reader.num_rows());
        reader.extract_properties(
            listIdxs.data(), verts_per_face, miniply::PLYPropertyType::Int, mesh.triangles.data());
        gotFaces = true;
      }
      reader.next_element();
    }

    benchmark::DoNotOptimize(mesh);
  }
}

BENCHMARK_CAPTURE(BM_ParseHapply, "Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply");
BENCHMARK_CAPTURE(BM_ParseMiniply, "Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply");
BENCHMARK_CAPTURE(BM_ParsePlywoot, "Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply");

BENCHMARK_CAPTURE(BM_ParseHapply, "Lucy (binary big endian)", "models/lucy.ply");
BENCHMARK_CAPTURE(BM_ParseMiniply, "Lucy (binary big endian)", "models/lucy.ply");
BENCHMARK_CAPTURE(BM_ParsePlywoot, "Lucy (binary big endian)", "models/lucy.ply");

BENCHMARK_CAPTURE(
    BM_ParseHapply,
    "DOOM Combat Scene (binary little endian)",
    "models/Doom combat scene.ply");
BENCHMARK_CAPTURE(
    BM_ParseMiniply,
    "DOOM Combat Scene (binary little endian)",
    "models/Doom combat scene.ply");
BENCHMARK_CAPTURE(
    BM_ParsePlywoot,
    "DOOM Combat Scene (binary little endian)",
    "models/Doom combat scene.ply");

BENCHMARK_CAPTURE(BM_ParseHapply, "Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_CAPTURE(BM_ParseMiniply, "Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_CAPTURE(BM_ParsePlywoot, "Dragon (ASCII)", "models/dragon_vrip.ply");

BENCHMARK_CAPTURE(BM_ParseHapply, "Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_CAPTURE(BM_ParseMiniply, "Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_CAPTURE(BM_ParsePlywoot, "Happy Buddha (ASCII)", "models/happy_vrip.ply");

BENCHMARK_CAPTURE(BM_ParseHapply, "Stanford bunny (ASCII)", "models/bun_zipper.ply");
BENCHMARK_CAPTURE(BM_ParseMiniply, "Stanford bunny (ASCII)", "models/bun_zipper.ply");
BENCHMARK_CAPTURE(BM_ParsePlywoot, "Stanford bunny (ASCII)", "models/bun_zipper.ply");

BENCHMARK_MAIN();
