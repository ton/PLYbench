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

static void BM_ParseNanoPly(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parseNanoPly(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with nanoply").data());
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

static void BM_ParsePlyLib(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parsePlyLib(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with plylib").data());
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

#define BENCHMARK_ALL(name, filename)                                                              \
  BENCHMARK_CAPTURE(BM_ParseHapply, (name), (filename))->Unit(benchmark::kMillisecond);            \
  BENCHMARK_CAPTURE(BM_ParseMiniply, (name), (filename))->Unit(benchmark::kMillisecond);           \
  BENCHMARK_CAPTURE(BM_ParseMshPly, (name), (filename))->Unit(benchmark::kMillisecond);            \
  BENCHMARK_CAPTURE(BM_ParseNanoPly, (name), (filename))->Unit(benchmark::kMillisecond);           \
  BENCHMARK_CAPTURE(BM_ParsePlywoot, (name), (filename))->Unit(benchmark::kMillisecond);           \
  BENCHMARK_CAPTURE(BM_ParsePlyLib, (name), (filename))->Unit(benchmark::kMillisecond);

BENCHMARK_ALL("Asian Dragon (binary b/e)", "models/xyzrgb_dragon.ply")
BENCHMARK_ALL("Lucy (binary b/e)", "models/lucy.ply");

BENCHMARK_ALL("DOOM Combat Scene (binary l/e)", "models/Doom combat scene.ply");

BENCHMARK_ALL("Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_ALL("Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_ALL("Stanford Bunny (ASCII)", "models/bun_zipper.ply");

BENCHMARK_MAIN();
