#include <catch2/catch_session.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include <iostream>

int main(int argc, char* argv[]) {
    Catch::Session session;

    int returnCode = session.applyCommandLine(argc, argv);
    if(returnCode != 0) {
        return returnCode;
    }

    // writing to session.configData() or session.Config() here
    // overrides command line args
    // only do this if you know you need to

    char* envShardCount = std::getenv("TEST_TOTAL_SHARDS");
    if(envShardCount != nullptr && session.configData().shardCount == 1) {
        session.configData().shardCount = std::stoul(envShardCount);
    }

    char* envShardIndex = std::getenv("TEST_SHARD_INDEX");
    if(envShardIndex != nullptr && session.configData().shardIndex == 0) {
        session.configData().shardIndex = std::stoul(envShardIndex);
    }

    char* envShardStatusFile = std::getenv("TEST_SHARD_STATUS_FILE");
    if(envShardStatusFile != nullptr) {
        if(std::filesystem::exists(envShardStatusFile)) {
            auto now           = std::chrono::system_clock::now();
            auto filesystemNow = std::filesystem::file_time_type(now.time_since_epoch());
            std::filesystem::last_write_time(envShardStatusFile, filesystemNow);
        } else {
            std::ofstream(envShardStatusFile);
        }
    }

    std::cout << "Shards: " << session.configData().shardCount << " Index: " << session.configData().shardIndex
              << std::endl;

    int numFailed = session.run();

    // numFailed is clamped to 255 as some unices only use the lower 8 bits.
    // This clamping has already been applied, so just return it here
    // You can also do any post run clean-up here
    return numFailed;
}
