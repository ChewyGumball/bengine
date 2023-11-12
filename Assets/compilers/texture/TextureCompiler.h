#pragma once

#include <Core/Status/Status.h>

#include <filesystem>

namespace Assets::TextureCompiler {
Core::Status Compile(const std::filesystem::path& inputFile, const std::filesystem::path& outputFile);
}
