#include "mesh.h"

#include "submodules/miniply/miniply.h"

#include <plywoot/plywoot.hpp>

#include <benchmark/benchmark.h>

#include <cstdint>
#include <fstream>
#include <vector>

static void BM_plywoot(benchmark::State &state, const std::string &filename)
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

    using VertexLayout = plywoot::reflect::Layout<float, float, float>;
    const std::vector<Vertex> vertices{plyIn.read<Vertex, VertexLayout>(vertexElement)};
    benchmark::DoNotOptimize(vertices);

    using TriangleLayout = plywoot::reflect::Layout<plywoot::reflect::Array<int, 3>>;
    const std::vector<Triangle> triangles{plyIn.read<Triangle, TriangleLayout>(faceElement)};
    benchmark::DoNotOptimize(triangles);
  }
}

static void BM_miniply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    const int verts_per_face = 3;

    miniply::PLYReader reader{filename.data()};
    if (!reader.valid()) { state.SkipWithError((std::string{"could not open: "} + filename).data()); }

    miniply::PLYElement *facesElem = reader.get_element(reader.find_element(miniply::kPLYFaceElement));
    if (!facesElem)
    {
      state.SkipWithError("could not load triangle data; face element not found in input data");
    }

    std::vector<std::uint32_t> listIdxs;
    listIdxs.resize(3);
    facesElem->convert_list_to_fixed_size(
        facesElem->find_property("vertex_indices"), verts_per_face, listIdxs.data());

    std::vector<Triangle> triangles;
    std::vector<Vertex> vertices;

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
        vertices.resize(reader.num_rows());
        reader.extract_properties(propIdxs, 3, miniply::PLYPropertyType::Float, vertices.data());
        gotVerts = true;
      }
      else if (!gotFaces && reader.element_is(miniply::kPLYFaceElement))
      {
        if (!reader.load_element())
        {
          state.SkipWithError("could not load triangle data; could not load face element");
        }

        triangles.resize(reader.num_rows());
        reader.extract_properties(
            listIdxs.data(), verts_per_face, miniply::PLYPropertyType::Int, triangles.data());
        gotFaces = true;
      }
      reader.next_element();
    }

    benchmark::DoNotOptimize(triangles);
    benchmark::DoNotOptimize(vertices);
  }
}

BENCHMARK_CAPTURE(BM_plywoot, "Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply");
BENCHMARK_CAPTURE(BM_miniply, "Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply");

BENCHMARK_CAPTURE(BM_plywoot, "Lucy (binary big endian)", "models/lucy.ply");
BENCHMARK_CAPTURE(BM_miniply, "Lucy (binary big endian)", "models/lucy.ply");

BENCHMARK_CAPTURE(BM_plywoot, "Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_CAPTURE(BM_miniply, "Dragon (ASCII)", "models/dragon_vrip.ply");

BENCHMARK_CAPTURE(BM_plywoot, "Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_CAPTURE(BM_miniply, "Happy Buddha (ASCII)", "models/happy_vrip.ply");

BENCHMARK_CAPTURE(BM_plywoot, "Stanford bunny (ASCII)", "models/bun_zipper.ply");
BENCHMARK_CAPTURE(BM_miniply, "Stanford bunny (ASCII)", "models/bun_zipper.ply");

BENCHMARK_CAPTURE(BM_plywoot, "DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply");
BENCHMARK_CAPTURE(BM_miniply, "DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply");

BENCHMARK_MAIN();
