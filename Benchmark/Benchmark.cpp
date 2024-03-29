// Benchmark.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <hayai/hayai.hpp>

#include "core/algorithms/Strings.h"
#include "core/containers/Array.h"



BENCHMARK(StringParsing, StringToIntCustom, 10, 1000) {
    Core::Algorithms::String::ParseInt64("63987987");
}
BENCHMARK(StringParsing, StringToFloatCustom, 10, 1000) {
    Core::Algorithms::String::ParseFloat("-838563.91887687668795965");
}

BENCHMARK(StringParsing, StringToIntSTD, 10, 1000) {
    std::stoll("63987987");
}
BENCHMARK(StringParsing, StringToFloatSTD, 10, 1000) {
    std::stof("-838563.91887687668795965");
}

BENCHMARK(VectorInsert, PushBackSTD, 10, 10) {
    std::vector<uint64_t> a;
    for(size_t i = 0; i < 1000000; i++) {
        a.push_back(i);
    }
}

BENCHMARK(VectorInsert, InsertCore, 10, 10) {
    Core::Array<uint64_t> a;
    for(size_t i = 0; i < 1000000; i++) {
        a.insert(i);
    }
}

BENCHMARK(VectorInsertFront, InsertSTD, 10, 10) {
    std::vector<uint64_t> a;
    for(size_t i = 0; i < 100000; i++) {
        a.insert(a.begin(), i);
    }
}

BENCHMARK(VectorInsertFront, InsertAtCore, 10, 10) {
    Core::Array<uint64_t> a;
    for(size_t i = 0; i < 100000; i++) {
        a.insertAt(0, i);
    }
}

int main() {
    hayai::ConsoleOutputter consoleOutputter;

    hayai::Benchmarker::AddOutputter(consoleOutputter);
    hayai::Benchmarker::RunAllTests();
    return 0;
}
