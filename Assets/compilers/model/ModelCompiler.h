
#include <Core/Status/Status.h>

#include <filesystem>

namespace Assets::ModelCompiler {
Core::Status Compile(const std::filesystem::path& inputFile, const std::filesystem::path& outputFile);
}
