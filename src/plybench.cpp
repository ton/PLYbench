#include "mesh.h"
#include "parsers.h"
#include "util.h"
#include "writers.h"

#include <benchmark/benchmark.h>

#include <cstdint>

namespace {
constexpr std::int32_t writeNumTriangles = 100000;

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

static void BM_WriteHapply(benchmark::State &state, Format format)
{
  benchmark::ClobberMemory();

  const TriangleMesh mesh{createMesh(writeNumTriangles)};
  for (auto _ : state) { writeHapply(mesh, format); }

  state.SetBytesProcessed(state.iterations() * meshSizeInBytes(mesh));
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

static void BM_WriteMshPly(benchmark::State &state, Format format)
{
  benchmark::ClobberMemory();

  const TriangleMesh mesh{createMesh(writeNumTriangles)};
  for (auto _ : state) { writeMshPly(mesh, format); }

  state.SetBytesProcessed(state.iterations() * meshSizeInBytes(mesh));
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

static void BM_WriteNanoPly(benchmark::State &state, Format format)
{
  benchmark::ClobberMemory();

  const TriangleMesh mesh{createMesh(writeNumTriangles)};
  for (auto _ : state) { writeNanoPly(mesh, format); }

  state.SetBytesProcessed(state.iterations() * meshSizeInBytes(mesh));
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

static void BM_WritePlywoot(benchmark::State &state, Format format)
{
  benchmark::ClobberMemory();

  const TriangleMesh mesh{createMesh(writeNumTriangles)};
  for (auto _ : state) { writePlywoot(mesh, format); }

  state.SetBytesProcessed(state.iterations() * meshSizeInBytes(mesh));
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

static void BM_ParseRPly(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parseRPly(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with RPly").data());
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

static void BM_WriteRPly(benchmark::State &state, Format format)
{
  benchmark::ClobberMemory();

  const TriangleMesh mesh{createMesh(writeNumTriangles)};
  for (auto _ : state) { writeRPly(mesh, format); }

  state.SetBytesProcessed(state.iterations() * meshSizeInBytes(mesh));
}

static void BM_ParseTinyply(benchmark::State &state, const std::string &filename)
{
  benchmark::ClobberMemory();

  std::optional<TriangleMesh> maybeMesh;
  for (auto _ : state)
  {
    if (!(maybeMesh = parseTinyply(filename)))
      state.SkipWithError((std::string{"could not parse '"} + filename + "' with tinyply").data());
  }

  if (maybeMesh) state.SetBytesProcessed(state.iterations() * meshSizeInBytes(*maybeMesh));
}

static void BM_WriteTinyply(benchmark::State &state, Format format)
{
  benchmark::ClobberMemory();

  const TriangleMesh mesh{createMesh(writeNumTriangles)};
  for (auto _ : state) { writeTinyply(mesh, format); }

  state.SetBytesProcessed(state.iterations() * meshSizeInBytes(mesh));
}

#define TIME_UNIT benchmark::kMillisecond

#define BENCHMARK_PARSE(name, filename)                                                                      \
  BENCHMARK_CAPTURE(BM_ParseHapply, name, (filename))->Unit(TIME_UNIT);                                      \
  BENCHMARK_CAPTURE(BM_ParseMiniply, name, (filename))->Unit(TIME_UNIT);                                     \
  BENCHMARK_CAPTURE(BM_ParseMshPly, name, (filename))->Unit(TIME_UNIT);                                      \
  BENCHMARK_CAPTURE(BM_ParseNanoPly, name, (filename))->Unit(TIME_UNIT);                                     \
  BENCHMARK_CAPTURE(BM_ParsePlywoot, name, (filename))->Unit(TIME_UNIT);                                     \
  BENCHMARK_CAPTURE(BM_ParsePlyLib, name, (filename))->Unit(TIME_UNIT);                                      \
  BENCHMARK_CAPTURE(BM_ParseRPly, name, (filename))->Unit(TIME_UNIT);                                        \
  BENCHMARK_CAPTURE(BM_ParseTinyply, name, (filename))->Unit(TIME_UNIT);

#define BENCHMARK_PARSE_NO_TINYPLY(name, filename)                                                           \
  BENCHMARK_CAPTURE(BM_ParseHapply, name, (filename))->Unit(TIME_UNIT);                                      \
  BENCHMARK_CAPTURE(BM_ParseMiniply, name, (filename))->Unit(TIME_UNIT);                                     \
  BENCHMARK_CAPTURE(BM_ParseMshPly, name, (filename))->Unit(TIME_UNIT);                                      \
  BENCHMARK_CAPTURE(BM_ParseNanoPly, name, (filename))->Unit(TIME_UNIT);                                     \
  BENCHMARK_CAPTURE(BM_ParsePlywoot, name, (filename))->Unit(TIME_UNIT);                                     \
  BENCHMARK_CAPTURE(BM_ParsePlyLib, name, (filename))->Unit(TIME_UNIT);                                      \
  BENCHMARK_CAPTURE(BM_ParseRPly, name, (filename))->Unit(TIME_UNIT);

BENCHMARK_PARSE("Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply")
BENCHMARK_PARSE("Lucy (binary big endian)", "models/lucy.ply");

BENCHMARK_PARSE("DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply");
BENCHMARK_PARSE("PBRT-v3 Dragon (binary little endian)", "models/dragon_remeshed.ply");

// Note; tinyply 2.3 seems to be broken for ASCII
// (https://github.com/ddiakopoulos/tinyply/issues/59)
BENCHMARK_PARSE_NO_TINYPLY("Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_PARSE_NO_TINYPLY("Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_PARSE_NO_TINYPLY("Stanford Bunny (ASCII)", "models/bun_zipper.ply");

#define BENCHMARK_WRITE(benchmarkName)                                                                       \
  BENCHMARK_CAPTURE(benchmarkName, "ASCII", Format::Ascii)->Unit(TIME_UNIT);                                 \
  BENCHMARK_CAPTURE(benchmarkName, "binary", Format::BinaryLittleEndian)->Unit(TIME_UNIT);

BENCHMARK_WRITE(BM_WriteHapply);
BENCHMARK_WRITE(BM_WriteMshPly);
BENCHMARK_WRITE(BM_WriteNanoPly);
BENCHMARK_WRITE(BM_WritePlywoot);
BENCHMARK_WRITE(BM_WriteRPly);
BENCHMARK_WRITE(BM_WriteTinyply);

BENCHMARK_MAIN();
