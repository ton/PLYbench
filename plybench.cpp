#include "mesh.h"
#include "parsers.h"

#include <benchmark/benchmark.h>

#include <cstdint>

static void BM_ParseHapply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    std::optional<TriangleMesh> maybeMesh{parseHapply(filename)};
    if (!maybeMesh)
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with hapPLY").data());
    benchmark::DoNotOptimize(maybeMesh);
  }
}

static void BM_ParseMiniply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    std::optional<TriangleMesh> maybeMesh{parseMiniply(filename)};
    if (!maybeMesh)
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with MiniPLY").data());
    benchmark::DoNotOptimize(maybeMesh);
  }
}

static void BM_ParsePlywoot(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  for (auto _ : state)
  {
    std::optional<TriangleMesh> maybeMesh{parsePlyWoot(filename)};
    if (!maybeMesh)
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with PLYwoot").data());
    benchmark::DoNotOptimize(maybeMesh);
  }
}

#define BENCHMARK_ALL(name, filename)                                                              \
  BENCHMARK_CAPTURE(BM_ParseHapply, (name), (filename));                                           \
  BENCHMARK_CAPTURE(BM_ParseMiniply, (name), (filename));                                          \
  BENCHMARK_CAPTURE(BM_ParsePlywoot, (name), (filename));

BENCHMARK_ALL("Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply")
BENCHMARK_ALL("Lucy (binary big endian)", "models/lucy.ply");

BENCHMARK_ALL("DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply");

BENCHMARK_ALL("Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_ALL("Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_ALL("Stanford bunny (ASCII)", "models/bun_zipper.ply");

BENCHMARK_MAIN();
