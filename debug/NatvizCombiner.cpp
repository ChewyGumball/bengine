#include "core/algorithms/Strings.h"
#include "core/io/file_system/FileSystem.h"
#include "core/io/serialization/OutputStream.h"
#include <Core/Logging/Logger.h>
#include <Core/Status/Status.h>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

#include <string_view>

inline Core::Status combine(const std::vector<std::filesystem::path>& inputFiles,
                            const std::filesystem::path& outputFile) {
    using namespace std::literals;

    ASSIGN_OR_RETURN(Core::IO::OutputStream out, Core::IO::OpenFileForWrite(outputFile));
    out.writeText("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"sv);
    out.writeText("<AutoVisualizer xmlns=\"http://schemas.microsoft.com/vstudio/debugger/natvis/2010\">\n"sv);

    for(auto inputFile : inputFiles) {
        ASSIGN_OR_RETURN(std::string contents, Core::IO::ReadTextFile(inputFile));
        Core::Array<std::string_view> lines =
              Core::Algorithms::String::SplitLines(contents, Core::Algorithms::String::Filter::Whitespace);

        // Skip the first two lines, and the last line because those are the xml version specification,
        // and root AutoVisualizer node tags. These lines are duplicated in every input file, and we only
        // can have them once in the output file.
        Core::Span<std::string_view> lineSpan = Core::ToSpan(lines).subspan(2, lines.count() - 3);
        for(auto line : lineSpan) {
            out.writeText(line);
            out.writeText("\n"sv);
        }
    }

    out.writeText("</AutoVisualizer>\n"sv);

    return Core::Status::Ok();
}

int main(int argc, char** argv) {
    Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Info);

    CLI::App app("Bengine Natvis Combiner");

    std::vector<std::filesystem::path> inputFiles;
    app.add_option("--input", inputFiles, "Input file to combine")->required()->check(CLI::ExistingFile);

    std::filesystem::path outputFile;
    app.add_option("--output", outputFile, "Path to write the combined output to")->required();

    app.add_flag_callback(
          "--quiet",
          []() { Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Error); },
          "Only print error messages");

    CLI11_PARSE(app, argc, argv);

    Core::Status status = combine(inputFiles, outputFile);

    if(status.isError()) {
        Core::LogCategory Combiner("Natvis Combiner");
        Core::Log::Error(Combiner, "Error combining natvis files: {}", status.message());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
