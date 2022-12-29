#pragma once

#include "mesh.h"

#include <optional>
#include <string>

std::optional<TriangleMesh> parseHapply(const std::string &filename);
std::optional<TriangleMesh> parseMiniply(const std::string &filename);
std::optional<TriangleMesh> parseMshPly(const std::string &filename);
std::optional<TriangleMesh> parsePlyWoot(const std::string &filename);
