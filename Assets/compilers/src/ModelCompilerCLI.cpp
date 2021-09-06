
#include <Assets/Compilers/ModelCompiler.h>

#include <Core/Logging/Logger.h>

#include <CLI/App.hpp>
#include <CLI/Config.hpp>
#include <CLI/Formatter.hpp>

int main(int argc, char** argv) {
    Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Info);

    CLI::App app("Bengine Model Compiler");

    std::filesystem::path inputFile;
    app.add_option("--input", inputFile, "Input file to compile")->required()->check(CLI::ExistingFile);

    std::filesystem::path outputFile;
    app.add_option("--output", outputFile, "Path to write the compiled output to")->required();

    app.add_flag_callback(
          "--quiet",
          []() { Core::LogManager::SetGlobalMinimumLevel(Core::LogLevel::Error); },
          "Only print error messages");

    CLI11_PARSE(app, argc, argv);

    Core::Status status = Assets::ModelCompiler::Compile(inputFile, outputFile);

    if(status.isError()) {
        Core::LogCategory Compiler("Model Compiler");
        Core::Log::Error(Compiler, "Error compiling model: {}", status.message());
        return EXIT_FAILURE;
    } else {
        return EXIT_SUCCESS;
    }
}
