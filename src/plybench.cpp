#include "mesh.h"
#include "parsers.h"
#include "util.h"
#include "writers.h"

#include <benchmark/benchmark.h>

#include <cstdint>

namespace
{
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

// Note; tinyply 2.3 seems to be broken for ASCII
// (https://github.com/ddiakopoulos/tinyply/issues/59)

#define BENCHMARK_PARSE(name, filename)                                                            \
  BENCHMARK_CAPTURE(BM_ParseHapply, name, (filename))->Unit(benchmark::kMillisecond);            \
  BENCHMARK_CAPTURE(BM_ParseMiniply, name, (filename))->Unit(benchmark::kMillisecond);           \
  BENCHMARK_CAPTURE(BM_ParseMshPly, name, (filename))->Unit(benchmark::kMillisecond);            \
  BENCHMARK_CAPTURE(BM_ParseNanoPly, name, (filename))->Unit(benchmark::kMillisecond);           \
  BENCHMARK_CAPTURE(BM_ParsePlywoot, name, (filename))->Unit(benchmark::kMillisecond);           \
  BENCHMARK_CAPTURE(BM_ParsePlyLib, name, (filename))->Unit(benchmark::kMillisecond);            \
  BENCHMARK_CAPTURE(BM_ParseRPly, name, (filename))->Unit(benchmark::kMillisecond);

BENCHMARK_PARSE("Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply")
BENCHMARK_CAPTURE(BM_ParseTinyply, "Asian Dragon (binary big endian)", "models/xyzrgb_dragon.ply")
    ->Unit(benchmark::kMillisecond);

BENCHMARK_PARSE("Lucy (binary big endian)", "models/lucy.ply");
BENCHMARK_CAPTURE(BM_ParseTinyply, "Lucy (binary big endian)", "models/lucy.ply")
    ->Unit(benchmark::kMillisecond);

BENCHMARK_PARSE("DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply");
BENCHMARK_CAPTURE(BM_ParseTinyply, "DOOM Combat Scene (binary little endian)", "models/Doom combat scene.ply")
    ->Unit(benchmark::kMillisecond);

BENCHMARK_PARSE("Dragon (ASCII)", "models/dragon_vrip.ply");
BENCHMARK_PARSE("Happy Buddha (ASCII)", "models/happy_vrip.ply");
BENCHMARK_PARSE("Stanford Bunny (ASCII)", "models/bun_zipper.ply");

BENCHMARK_CAPTURE(BM_WriteHapply, "ASCII", Format::Ascii)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteHapply, "Binary", Format::BinaryLittleEndian)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteMshPly, "ASCII", Format::Ascii)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteMshPly, "Binary", Format::BinaryLittleEndian)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteNanoPly, "ASCII", Format::Ascii)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteNanoPly, "Binary", Format::BinaryLittleEndian)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WritePlywoot, "ASCII", Format::Ascii)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WritePlywoot, "Binary", Format::BinaryLittleEndian)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteRPly, "ASCII", Format::Ascii)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteRPly, "Binary", Format::BinaryLittleEndian)
    ->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteTinyply, "ASCII", Format::Ascii)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(BM_WriteTinyply, "Binary", Format::BinaryLittleEndian)
    ->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
