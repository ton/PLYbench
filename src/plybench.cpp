#include "mesh.h"
#include "parsers.h"

#include <benchmark/benchmark.h>

#include <cstdint>

namespace
{
  std::size_t meshSizeInBytes(const TriangleMesh &mesh)
  {
    return mesh.triangles.size() * sizeof(Triangle) + mesh.vertices.size() * sizeof(Vertex);
  }
}

static void BM_ParseHapply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parseHapply(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with hapPLY").data());
    benchmark::DoNotOptimize(maybeMesh);
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

static void BM_ParseMiniply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parseMiniply(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with MiniPLY").data());
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

static void BM_ParseMshPly(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parseMshPly(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with msh_ply").data());
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

static void BM_ParsePlywoot(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parsePlywoot(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with PLYwoot").data());
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

#define BENCHMARK_ALL(name, filename)                                                              \
  BENCHMARK_CAPTURE(BM_ParseHapply, (name), (filename));                                           \
  BENCHMARK_CAPTURE(BM_ParseMiniply, (name), (filename));                                          \
  BENCHMARK_CAPTURE(BM_ParseMshPly, (name), (filename));                                           \
  BENCHMARK_CAPTURE(BM_ParsePlywoot, (name), (filename));

BENCHMARK_ALL("Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply")
BENCHMARK_ALL("Lucy (binary big endian)", "models/lucy.ply");

BENCHMARK_ALL("DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply");

BENCHMARK_ALL("Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_ALL("Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_ALL("Stanford bunny (ASCII)", "models/bun_zipper.ply");

BENCHMARK_MAIN();
