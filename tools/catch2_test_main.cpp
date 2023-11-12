#include <catch2/catch_session.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <iostream>

int main(int argc, char* argv[]) {
    return Catch::Session().run(argc, argv);
}
